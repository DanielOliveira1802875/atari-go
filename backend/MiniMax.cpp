#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include "unordered_dense.h"
#include "AtariGo.h"


inline Player opponent(const Player p) { return p == BLACK ? WHITE : BLACK; }
constexpr int INF = WIN * 2;
constexpr uint64_t CHECK_INTERVAL = 10'000;

inline int distanceAwareScore(const int raw, const int ply) {
    if (raw >= WIN) return raw - ply; // sooner win  -> bigger value
    if (raw <= -WIN) return raw + ply; // later loss -> less negative
    return raw; // heuristic, leave untouched
}


class MiniMax {
private:
    // Transposition table: signature -> (score, depth, bound)
    mutable ankerl::unordered_dense::map<uint64_t, std::tuple<int, int, Bound> > transpositionTable;

    struct SearchContext {
        const std::chrono::steady_clock::time_point start;
        const std::chrono::milliseconds timeLimit;
        bool timedOut = false;
        uint64_t nodeCount = 0;
    };

    int minimax(const Board &state, int depth, int alpha, int beta, SearchContext &ctx, const int ply) const {
        // Check if the search has timed out
        if (ctx.timedOut) return 0;

        // Increment node count and occasionally check time
        if (++ctx.nodeCount % CHECK_INTERVAL == 0) {
            if (std::chrono::steady_clock::now() - ctx.start >= ctx.timeLimit) {
                ctx.timedOut = true;
                return 0;
            }
        }

        // Check transposition table
        const uint64_t signature = state.getSignature();
        if (const auto it = transpositionTable.find(signature); it != transpositionTable.end()) {
            auto &[score, storedDepth, flag] = it->second;
            if (storedDepth >= depth) {
                switch (flag) {
                    case EXACT: return score;
                    case LOWER: alpha = std::max(alpha, score); break;
                    case UPPER: beta = std::min(beta, score); break;
                }
                if (alpha >= beta) return score;
            } else if (std::abs(score) >= (WIN - 20)) {
                // If the score is a win or a loss, the depth is irrelevant. The -20 accounts for the ply value.
                    return score;
            }
        }

        // Check if the game is over or if we reached the maximum depth
        if (depth == 0 || AtariGo::isTerminal(state)) return distanceAwareScore(state.getHeuristic(), ply);

        // Generate successors
        const auto successors = AtariGo::generateSuccessors(state);
        if (successors.empty()) return distanceAwareScore(state.getHeuristic(), ply);

        const Player toMove = state.getPlayerToMove();
        int best = toMove == WHITE ? -INF : INF;
        const int origAlpha = alpha;
        const int origBeta = beta;

        // Iterate through successors
        for (auto &child: successors) {
            int score = minimax(child, depth - 1, alpha, beta, ctx, ply + 1);
            if (ctx.timedOut) return 0;

            if (toMove == WHITE) {
                best = std::max(best, score);
                alpha = std::max(alpha, best);
            } else {
                best = std::min(best, score);
                beta = std::min(beta, best);
            }
            if (alpha >= beta) break;
        }

        // Store the best score in the transposition table
        Bound flag = EXACT;
        if (best <= origAlpha) flag = UPPER;
        else if (best >= origBeta) flag = LOWER;
        transpositionTable[signature] = {best, depth, flag};

        return best;
    }

public:
    Board getBestMove(const Board &state, const std::chrono::milliseconds timeLimit, const int depthLimit) const {
        static std::mt19937_64 rng{std::random_device{}()};
        SearchContext ctx{std::chrono::steady_clock::now(), timeLimit};

        std::vector<Board> successors = AtariGo::generateSuccessors(state);

        // For each strong move, if it is empty and has no neighbors, add it to the successors.
        std::vector<Board> strongMoves;
        const auto occupation = state.getOccupiedBits();
        for (const Bitboard128 i: STRONG_MOVE_MASK) {
            if (i == 0) break;
            const auto index = getLSBIndex(i);
            if (state.isEmpty(index)) {
                const auto liberties = getLibertyBits(i, occupation);
                if (bitCount(liberties) != 4) continue;
                auto successor = state;
                successor.setStone(index);
                AtariGo::computeHeuristic(successor);
                strongMoves.push_back(successor);
            }
        }

        // add a random strong move to the successors
        if (!strongMoves.empty()) {
            std::uniform_int_distribution<int> dist(0, strongMoves.size() - 1);
            successors.push_back(strongMoves[dist(rng)]);
        }

        // always add the center
        if (state.isEmpty(BOARD_EDGE / 2, BOARD_EDGE / 2)) {
            Board center = state;
            center.setStone(BOARD_EDGE / 2, BOARD_EDGE / 2);
            AtariGo::computeHeuristic(center);
            successors.push_back(center);
        }


        if (successors.empty()) {
            std::cerr << "Warning: No successors generated from the current state.\n";
            return state;
        }
        if (successors.size() == 1) return successors[0];

        transpositionTable.clear();
        transpositionTable.reserve(10'000'000);
        int overallBestScore = 0;
        std::vector<int> overallBestIdx;

        const Player currentPlayer = state.getPlayerToMove();

        for (int depth = 1; depth <= depthLimit && !ctx.timedOut; ++depth) {
            int bestScore = (currentPlayer == BLACK) ? INF : -INF; // For WHITE (Maximizer) this is alpha, for BLACK (Minimizer) this is beta
            std::vector<int> bestIdx;

            // Process the first successor
            if (!successors.empty()) {
                // For the first child, use a full alpha-beta window
                bestScore = minimax(successors[0], depth - 1, -INF, INF, ctx, 0);
                if (ctx.timedOut) break;
                bestIdx = {0};
            }

            // Process remaining successors
            for (int i = 1; i < static_cast<int>(successors.size()) && !ctx.timedOut; ++i) {
                int score;
                if (currentPlayer == WHITE) {
                    // Alpha is the bestScore found so far from previous siblings, Beta is INF
                    score = minimax(successors[i], depth - 1, bestScore, INF, ctx, 0);
                } else { // BLACK
                    // Alpha is -INF, Beta is the bestScore found so far from previous siblings
                    score = minimax(successors[i], depth - 1, -INF, bestScore, ctx, 0);
                }

                if (ctx.timedOut) break;

                // Update best score and best move(s) for the current depth
                if (currentPlayer == WHITE) {
                    if (score > bestScore) {
                        bestScore = score; // Update alpha
                        bestIdx = {i};
                    } else if (score == bestScore) {
                        bestIdx.push_back(i);
                    }
                } else { // BLACK
                    if (score < bestScore) {
                        bestScore = score; // Update beta
                        bestIdx = {i};
                    } else if (score == bestScore) {
                        bestIdx.push_back(i);
                    }
                }
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
                        << ". TT Size: " << transpositionTable.size()
                        << ". Time: " << elapsedMs << " ms\n";

                if (std::abs(overallBestScore) >= WIN) break;
            }
        }

        if (overallBestIdx.empty()) {
            std::cerr << "Error: No best move identified after search. Returning first successor.\n";
            // Fallback to first successor if search was interrupted early or no valid moves found by search logic
            if (!successors.empty()) {
                 return successors[0];
            }
            // If successors is also empty (should be caught earlier, but as a safeguard)
            std::cerr << "Critical Error: No successors and overallBestIdx is empty. Returning original state.\n";
            return state;
        }

        std::uniform_int_distribution<int> dist(0, overallBestIdx.size() - 1);
        return successors[overallBestIdx[dist(rng)]];
    }
};
