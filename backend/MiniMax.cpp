#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include "AtariGo.h"


inline Player opponent(Player p) { return p == BLACK ? WHITE : BLACK; }
constexpr int INF = WIN * 2;
constexpr uint64_t CHECK_INTERVAL = 10'000;


class MiniMax {
    struct SearchContext {
        const std::chrono::steady_clock::time_point start;
        const std::chrono::milliseconds timeLimit;
        bool timedOut = false;
        uint64_t nodeCount = 0;
    };

    int minimax(Board &state, const int depth, int alpha, int beta, SearchContext &ctx) const {
        if (ctx.timedOut) return 0;

        if (++ctx.nodeCount % CHECK_INTERVAL == 0) {
            if (std::chrono::steady_clock::now() - ctx.start >= ctx.timeLimit) {
                ctx.timedOut = true;
                return 0;
            }
        }

        if (depth == 0 || AtariGo::isTerminal(state)) {
            AtariGo::calculateHeuristic(state);
            return state.getHeuristic();
        }

        auto successors = AtariGo::generateSuccessors(state);

        if (successors.empty()) {
            AtariGo::calculateHeuristic(state);
            return state.getHeuristic();
        }

        const Player toMove = state.getPlayerToMove();
        int best = toMove == WHITE ? -INF : INF;

        for (auto &child: successors) {
            int score = minimax(child, depth - 1, alpha, beta, ctx);
            if (ctx.timedOut) return 0;

            if (toMove == WHITE) {
                best = std::max(best, score);
                alpha = std::max(alpha, best);
            } else {
                best = std::min(best, score);
                beta = std::min(beta, best);
            }

            if (alpha >= beta) {
                break;
            }
        }

        return best;
    }

public:
    Board getBestMove(Board &state, const std::chrono::milliseconds timeLimit, const int depthLimit) const {
        static std::mt19937_64 rng{std::random_device{}()};
        SearchContext ctx{std::chrono::steady_clock::now(), timeLimit};

        std::vector<Board> successors = AtariGo::generateSuccessors(state);

        if (successors.empty()) {
            std::cerr << "Warning: No successors generated from the current state.\n";
            return state;
        }

        if (successors.size() == 1) return successors[0];

        int overallBestScore = 0;
        std::vector<int> overallBestIdx;

        const Player currentPlayer = state.getPlayerToMove();

        for (int depth = 1; depth <= depthLimit && !ctx.timedOut; ++depth) {
            int bestScore = (currentPlayer == BLACK) ? INF : -INF;
            std::vector<int> bestIdx;

            for (int i = 0; i < static_cast<int>(successors.size()) && !ctx.timedOut; ++i) {
                int score;
                if (currentPlayer == WHITE) {
                    score = minimax(successors[i], depth - 1, bestScore, INF, ctx);
                    if (score > bestScore) {
                        bestScore = score;
                        bestIdx = {i};
                    } else if (score == bestScore) {
                        bestIdx.push_back(i);
                    }
                } else { // BLACK
                    score = minimax(successors[i], depth - 1, -INF, bestScore, ctx);
                     if (score < bestScore) {
                        bestScore = score;
                        bestIdx = {i};
                    } else if (score == bestScore) {
                        bestIdx.push_back(i);
                    }
                }

                if (ctx.timedOut) break;
            }

            if (!ctx.timedOut && !bestIdx.empty()) {
                overallBestScore = bestScore;
                overallBestIdx = bestIdx;

                const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - ctx.start).count();

                std::cout << "Completed depth " << depth
                        << ". Best score: " << overallBestScore
                        << ". Candidates: " << bestIdx.size()
                        << ". Nodes: " << ctx.nodeCount
                        << ". Time: " << elapsedMs << " ms\n";

                if (std::abs(overallBestScore) >= WIN) break;
            }
        }

        if (overallBestIdx.empty()) {
            std::cerr << "Error: No best move identified after search. Returning first successor.\n";
            return successors[0];
        }

        std::uniform_int_distribution<int> dist(0, overallBestIdx.size() - 1);
        return successors[overallBestIdx[dist(rng)]];
    }
};