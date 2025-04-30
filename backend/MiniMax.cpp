#include <vector>
#include "unordered_dense.h"
#include <random>
#include "AtariGo.h"
#include <iostream>
#include <chrono>

enum Bound { EXACT, LOWER, UPPER };

class MiniMax {
private:
    static constexpr int INF = WIN * 2;

    mutable ankerl::unordered_dense::map<uint64_t, std::tuple<int, int, Bound> > transTable;

    static Player getOpponent(Player p) {
        return p == BLACK ? WHITE : BLACK;
    }

    int minimax(Board &state, int depth, int alpha, int beta,
                bool &time_up,
                const std::chrono::steady_clock::time_point &startTime,
                const std::chrono::milliseconds &timeLimit) const {

        const Player current = state.getPlayerToMove();
        // --- Time Check ---
        if (time_up || std::chrono::steady_clock::now() - startTime >= timeLimit) {
            time_up = true;
            return (current == WHITE ? alpha : beta);
        }

        uint64_t sig = state.getSignature();

        if (auto it = transTable.find(sig); it != transTable.end()) {
            auto &[score, storedDepth, flag] = it->second;
            if (storedDepth >= depth) {
                // apply bound logic
                switch (flag) {
                    case EXACT:
                        return score;
                    case LOWER:
                        alpha = std::max(alpha, score);
                        break;
                    case UPPER:
                        beta = std::min(beta, score);
                        break;
                    default: ;
                }
                if (alpha >= beta) return score;
            }
        }

        if (depth == 0 || AtariGo::isTerminal(state))
            return state.getHeuristic();

        auto successors = AtariGo::generateSuccessors(state);
        if (successors.empty())
            return state.getHeuristic();

        const int origAlpha = alpha;
        const int origBeta = beta;
        int bestValue = (current == WHITE) ? -INF : INF;

        for (auto &s: successors) {
            int eval = minimax(
                s, depth - 1,
                alpha, beta,
                time_up, startTime, timeLimit
            );
            if (time_up) return 0;

            if (current == WHITE) {
                bestValue = std::max(bestValue, eval);
                alpha = std::max(alpha, bestValue);
            } else {
                bestValue = std::min(bestValue, eval);
                beta = std::min(beta, bestValue);
            }
            if (alpha >= beta)
                break;
        }

        if (!time_up) {
            Bound flag;
            if (bestValue >= origBeta) flag = LOWER;
            else if (bestValue <= origAlpha) flag = UPPER;
            else flag = EXACT;
            transTable[sig] = {bestValue, depth, flag};
        }

        return bestValue;
    }

public:
    Board getBestMove(const Board &state, const std::chrono::milliseconds timeLimit, const int depthLimit) const {
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

        int overallBestScore = (state.getPlayerToMove() == BLACK) ? INF : -INF;
        int overallBestTurn = INF;
        std::vector<int> overallBestIndices;
        int deepestCompletedDepth = 0;

        transTable.clear();

        // Iterative Deepening Loop
        for (int currentDepth = 1; currentDepth <= depthLimit; ++currentDepth) {

            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime);
            if (elapsedTime >= timeLimit) {
                std::cout << "Time limit reached. Stopping search." << std::endl;
                break;
            }

            auto iterationStartTime = std::chrono::steady_clock::now();
            bool time_up_for_this_depth = false;

            int bestScoreThisDepth = (state.getPlayerToMove() == BLACK) ? INF : -INF;
            int bestTurnThisDepth = 0;
            std::vector<int> bestIndicesThisDepth;

            // Evaluate successors at the current depth
            for (int i = 0; i < (int) successors.size(); ++i) {
                Board &successor = successors[i];

                int score = minimax(successor, currentDepth - 1, -INF, INF,
                                    time_up_for_this_depth, startTime, timeLimit);

                if (time_up_for_this_depth) break;

                int turn = successor.getTurn();

                bool isBetter;
                if (state.getPlayerToMove() == BLACK) isBetter = score < bestScoreThisDepth;
                else isBetter = score > bestScoreThisDepth;

                if (isBetter) {
                    bestScoreThisDepth = score;
                    bestTurnThisDepth = turn;
                    bestIndicesThisDepth.clear();
                    bestIndicesThisDepth.push_back(i);
                } else if (score == bestScoreThisDepth) {
                    bestIndicesThisDepth.push_back(i);
                }
            }

            if (time_up_for_this_depth) {
                std::cout << "Time limit reached during depth " << currentDepth << ". Using results from depth " << deepestCompletedDepth << "." << std::endl;
                break;
            }

            if (!bestIndicesThisDepth.empty()) {
                overallBestScore = bestScoreThisDepth;
                overallBestTurn = bestTurnThisDepth;
                overallBestIndices = bestIndicesThisDepth;
                deepestCompletedDepth = currentDepth;

                elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime);
                auto iterationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - iterationStartTime);

                std::cout << "Completed depth " << currentDepth
                        << ". Best score: " << overallBestScore
                        << " (Turn: " << overallBestTurn << ")"
                        << ". Candidates: " << overallBestIndices.size()
                        << ". Time: " << elapsedTime.count() << "ms"
                        << " (Iter: " << iterationDuration.count() << "ms)" << std::endl;
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
            if (!successors.empty()) return successors[0];
            return state;
        }

        std::cout << "Final best move selection from depth " << deepestCompletedDepth
                << ". Score: " << overallBestScore
                << ", Turn: " << overallBestTurn
                << ", Candidates: " << overallBestIndices.size() << std::endl;


        // Randomly select among the best moves found at the deepest completed depth
        static std::mt19937_64 rng{std::random_device{}()};
        std::uniform_int_distribution<int> dist(0, overallBestIndices.size() - 1);
        const int chosenIndex = overallBestIndices[dist(rng)];

        return successors[chosenIndex];
    }
};
