#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include "unordered_dense.h"
#include "AtariGo.h"

enum Bound { EXACT, LOWER, UPPER };

// Exception used to signal time cutoff
struct TimeUpException {};

class MiniMax {
private:
    static constexpr int INF = WIN * 2;

    // Transposition table: signature -> (score, depth, bound)
    mutable ankerl::unordered_dense::map<uint64_t, std::tuple<int, int, Bound>> transTable;

    // Node counter for throttled clock checks
    mutable uint64_t nodeCount = 0;
    static constexpr uint64_t CHECK_INTERVAL = 2048;  // check clock every 2048 nodes

    static Player getOpponent(Player p) {
        return p == BLACK ? WHITE : BLACK;
    }

    int minimax(Board &state,
                int depth,
                int alpha,
                int beta,
                const std::chrono::steady_clock::time_point &startTime,
                const std::chrono::milliseconds &timeLimit) const {

        // Increment node count and occasionally check time
        if (++nodeCount % CHECK_INTERVAL == 0) {
            if (std::chrono::steady_clock::now() - startTime >= timeLimit) {
                throw TimeUpException();
            }
        }

        uint64_t sig = state.getSignature();
        auto it = transTable.find(sig);
        if (it != transTable.end()) {
            auto &[score, storedDepth, flag] = it->second;
            if (storedDepth >= depth) {
                switch (flag) {
                    case EXACT:
                        return score;
                    case LOWER:
                        alpha = std::max(alpha, score);
                        break;
                    case UPPER:
                        beta = std::min(beta, score);
                        break;
                }
                if (alpha >= beta) return score;
            }
        }

        if (depth == 0 || AtariGo::isTerminal(state)) {
            return state.getHeuristic();
        }

        auto successors = AtariGo::generateSuccessors(state);
        if (successors.empty()) {
            return state.getHeuristic();
        }

        const Player current = state.getPlayerToMove();
        int origAlpha = alpha;
        int origBeta = beta;
        int bestValue = (current == WHITE) ? -INF : INF;

        for (auto &s : successors) {
            int eval = minimax(s, depth - 1, alpha, beta, startTime, timeLimit);
            if (current == WHITE) {
                bestValue = std::max(bestValue, eval);
                alpha = std::max(alpha, bestValue);
            } else {
                bestValue = std::min(bestValue, eval);
                beta = std::min(beta, bestValue);
            }
            if (alpha >= beta) break;
        }

        // Store in transposition table
        Bound flag;
        if (bestValue >= origBeta) flag = LOWER;
        else if (bestValue <= origAlpha) flag = UPPER;
        else flag = EXACT;
        transTable[sig] = {bestValue, depth, flag};

        return bestValue;
    }

public:
    Board getBestMove(const Board &state,
                      const std::chrono::milliseconds timeLimit,
                      const int depthLimit) const {
        auto startTime = std::chrono::steady_clock::now();

        std::vector<Board> successors = AtariGo::generateSuccessors(state);
        if (successors.empty()) {
            std::cerr << "Warning: No successors generated from the current state." << std::endl;
            return state;
        }

        if (successors.size() == 1) {
            std::cout << "Only one possible move." << std::endl;
            return successors[0];
        }

        int overallBestScore;
        std::vector<int> overallBestIndices;
        int deepestCompletedDepth = 0;

        transTable.clear();

        for (int currentDepth = 1; currentDepth <= depthLimit; ++currentDepth) {

            auto elapsed = std::chrono::steady_clock::now() - startTime;
            if (elapsed >= timeLimit) {
                std::cout << "Time limit reached. Stopping search." << std::endl;
                break;
            }

            int bestScoreThisDepth = (state.getPlayerToMove() == BLACK) ? INF : -INF;
            std::vector<int> bestIndicesThisDepth;

            try {
                for (int i = 0; i < (int)successors.size(); ++i) {
                    Board &succ = successors[i];
                    int score = minimax(succ,
                                        currentDepth - 1,
                                        -INF, INF,
                                        startTime, timeLimit);
                    if (state.getPlayerToMove() == BLACK) {
                        if (score < bestScoreThisDepth) {
                            bestScoreThisDepth = score;
                            bestIndicesThisDepth = {i};
                        } else if (score == bestScoreThisDepth) {
                            bestIndicesThisDepth.push_back(i);
                        }
                    } else {
                        if (score > bestScoreThisDepth) {
                            bestScoreThisDepth = score;
                            bestIndicesThisDepth = {i};
                        } else if (score == bestScoreThisDepth) {
                            bestIndicesThisDepth.push_back(i);
                        }
                    }
                }
            } catch (TimeUpException &) {
                std::cout << "Time limit reached during depth " << currentDepth
                          << ". Using results from depth " << deepestCompletedDepth << "." << std::endl;
                break;
            }

            auto elapsedTime = std::chrono::steady_clock::now() - startTime;
            if (!bestIndicesThisDepth.empty()) {
                overallBestScore = bestScoreThisDepth;
                overallBestIndices = bestIndicesThisDepth;
                deepestCompletedDepth = currentDepth;
                std::cout << "Completed depth " << currentDepth
                        << ". Best score: " << overallBestScore
                        << ". Candidates: " << overallBestIndices.size()
                        << ". Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count() << " ms" << std::endl;
            } else {
                std::cerr << "Warning: No best move found at depth " << currentDepth << std::endl;
                break;
            }

            if (abs(overallBestScore) >= WIN) {
                std::cout << "Found terminal score (" << overallBestScore << ") at depth " << currentDepth << ". Stopping early." << std::endl;
                break;
            }
        }

        if (overallBestIndices.empty()) {
            std::cerr << "Error: No best move identified after search. Returning first successor." << std::endl;
            return successors.empty() ? state : successors[0];
        }

        // Randomly select among the best
        static std::mt19937_64 rng{std::random_device{}()};
        std::uniform_int_distribution<int> dist(0, overallBestIndices.size() - 1);
        return successors[overallBestIndices[dist(rng)]];
    }
};
