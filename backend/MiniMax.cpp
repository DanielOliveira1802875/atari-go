#include <vector>
#include <iostream>
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

        // Check if the game is over or if we reached the maximum depth
        if (depth == 0 || AtariGo::isTerminal(state)) return distanceAwareScore(state.getHeuristic(), ply);

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
            if (std::abs(score) >= (WIN - 20)) {
                // If the score is a win or a loss, the depth is irrelevant. The -20 accounts for the ply value.
                return score;
            }
            if (storedDepth >= depth) {
                switch (flag) {
                    case EXACT: return score;
                    case LOWER: alpha = std::max(alpha, score); break;
                    case UPPER: beta = std::min(beta, score); break;
                }
                if (alpha >= beta) return score;
            }
        }

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
        SearchContext ctx{std::chrono::steady_clock::now(), timeLimit};

        // This will make the lower depths more accessible.
        if (depthLimit <= 2) {
            std::cout << "Warning: Removing 80% of successors.\n";
            AtariGo::removeRandomSuccessorsPercentage = 80;
        }
        else if (depthLimit <= 3) {
            std::cout << "Warning: Removing 50% of successors.\n";
            AtariGo::removeRandomSuccessorsPercentage = 50;
        }
        else if (depthLimit <= 4) {
            std::cout << "Warning: Removing 20% of successors.\n";
            AtariGo::removeRandomSuccessorsPercentage = 20;
        }
        else AtariGo::removeRandomSuccessorsPercentage = 0;

        std::vector<Board> successors = AtariGo::generateSuccessors(state);

        // For each opening move, if it is empty and has no neighbors, add it to the successors.
        std::vector<Board> openingMoves;
        const auto occupation = state.getOccupiedBits();
        for (const Bitboard128 i: OPENING_MOVE_MASK) {
            if (i == 0) break;
            const auto index = getLSBIndex(i);
            if (state.isEmpty(index)) {
                const auto liberties = getLibertyBits(i, occupation);
                if (bitCount(liberties) != 4) continue;
                auto successor = state;
                successor.setStone(index);
                AtariGo::computeHeuristic(successor);
                openingMoves.push_back(successor);
            }
        }

        // add a random opening move to the successors
        if (!openingMoves.empty()) {
            std::uniform_int_distribution<int> dist(0, openingMoves.size() - 1);
            successors.push_back(openingMoves[dist(getRandom())]);
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

            // Process remaining successors
            for (int i = 0; i < static_cast<int>(successors.size()) && !ctx.timedOut; ++i) {
                int score;
                if (currentPlayer == WHITE) {
                    // Alpha is the bestScore found so far from previous siblings, Beta is INF
                    score = minimax(successors[i], depth - 1, bestScore - 1, INF, ctx, 0);
                } else { // BLACK
                    // Alpha is -INF, Beta is the bestScore found so far from previous siblings
                    score = minimax(successors[i], depth - 1, -INF, bestScore + 1, ctx, 0);
                }

                if (ctx.timedOut) break;

                // Update best score and best move(s) for the current depth
                if (currentPlayer == WHITE) {
                    if (score > bestScore || bestIdx.empty()) {
                        bestScore = score; // Update alpha
                        bestIdx = {i};
                    } else if (score == bestScore) {
                        bestIdx.push_back(i);
                    }
                } else { // BLACK
                    if (score < bestScore || bestIdx.empty()) {
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
            if (!successors.empty()) return successors[0];

            // If successors is also empty (should be caught earlier, but as a safeguard)
            std::cerr << "Critical Error: No successors and overallBestIdx is empty. Returning original state.\n";
            return state;
        }

        std::uniform_int_distribution<int> dist(0, overallBestIdx.size() - 1);
        return successors[overallBestIdx[dist(getRandom())]];
    }
};
