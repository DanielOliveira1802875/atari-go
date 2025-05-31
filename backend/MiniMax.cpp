#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include "unordered_dense.h"
#include "AtariGo.h"


inline Player opponent(Player p) { return p == BLACK ? WHITE : BLACK; }
constexpr int INF = WIN * 2;
constexpr uint64_t CHECK_INTERVAL = 10'000;

inline int distanceAwareScore(int raw, int ply) {
    // raw ==  ±WIN  for true terminal wins/losses
    if (raw >=  WIN) return raw - ply;   // sooner win  -> bigger value
    if (raw <= -WIN) return raw + ply;   // later loss -> less negative
    return raw;                          // heuristic, leave untouched
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

    int minimax(Board &state, int depth, int alpha, int beta, SearchContext &ctx, const int ply) const {

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
        uint64_t sig = state.getSignature();
        if (const auto it = transpositionTable.find(sig); it != transpositionTable.end()) {
            auto &[score, storedDepth, flag] = it->second;
            if (storedDepth >= depth) {
                switch (flag) {
                    case EXACT: return score;
                    case LOWER: alpha = std::max(alpha, score);
                        break;
                    case UPPER: beta = std::min(beta, score);
                        break;
                }
                if (alpha >= beta) return score;
            }
        }

        // Check if the game is over or if we reached the maximum depth
        if (depth == 0 || AtariGo::isTerminal(state))  return distanceAwareScore(state.getHeuristic(), ply);

        // Generate successors
        auto succ = AtariGo::generateSuccessors(state);
        if (succ.empty()) return state.getHeuristic();

        const Player toMove = state.getPlayerToMove();
        int best = (toMove == WHITE) ? -INF : INF;
        int origAlpha = alpha, origBeta = beta;

        // Iterate through successors
        for (auto &child: succ) {
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
        Bound flag = Bound::EXACT;
        if (best <= origAlpha) flag = Bound::UPPER;
        else if (best >= origBeta) flag = Bound::LOWER;
        transpositionTable[sig] = {best, depth, flag};

        return best;
    }

public:
    Board getBestMove(const Board &state, std::chrono::milliseconds timeLimit, int depthLimit) const {
        SearchContext ctx{std::chrono::steady_clock::now(), timeLimit};

        if (!state.getIsHeuristicCalculated()) {
            throw std::runtime_error("Heuristic not calculated. Call AtariGo::calculateHeuristic() first.");
        }

        std::vector<Board> successors = AtariGo::generateSuccessors(state);

        // For each strong move, if it is empty and has no neighbors, add it to the successors.
        std::vector<Board> strongMoves;
        const auto occupation = state.getOccupiedBits();
        for (int i = 0; i < STRONG_MOVE_MASK.size(); ++i) {
            const auto index = getLSBIndex(STRONG_MOVE_MASK[i]);
            if (state.isEmpty(index)) {
                const auto liberties = getLibertyBits(STRONG_MOVE_MASK[i], occupation);
                if (bitCount(liberties) != 4) continue;
                auto successor = state;
                successor.setStone(index);
                AtariGo::calculateHeuristic(successor);
                strongMoves.push_back(successor);
            }
        }

        // add a random strong move to the successors
        if (!strongMoves.empty()) {
            static std::mt19937_64 rng{std::random_device{}()};
            std::uniform_int_distribution<int> dist(0, strongMoves.size() - 1);
            successors.push_back(strongMoves[dist(rng)]);
        }

        // allways add the center
        if (state.isEmpty(BOARD_EDGE / 2, BOARD_EDGE / 2)) {
            Board center = state;
            center.setStone(BOARD_EDGE / 2, BOARD_EDGE / 2);
            AtariGo::calculateHeuristic(center);
            successors.push_back(center);
        }


        if (successors.empty()) {
            std::cerr << "Warning: No successors generated from the current state.\n";
            return state;
        }
        if (successors.size() == 1) return successors[0];

        transpositionTable.clear();
        int overallBestScore = 0;
        std::vector<int> overallBestIdx;

        for (int depth = 1; depth <= depthLimit && !ctx.timedOut; ++depth) {
            int bestScore = (state.getPlayerToMove() == BLACK) ? INF : -INF;
            std::vector<int> bestIdx;

            for (int i = 0; i < static_cast<int>(successors.size()) && !ctx.timedOut; ++i) {
                const int score = minimax(successors[i], depth - 1, -INF, INF, ctx, 0);
                if (ctx.timedOut) break;

                if (state.getPlayerToMove() == BLACK) {
                    if (score < bestScore) {
                        bestScore = score;
                        bestIdx = {i};
                    } else if (score == bestScore) bestIdx.push_back(i);
                } else {
                    if (score > bestScore) {
                        bestScore = score;
                        bestIdx = {i};
                    } else if (score == bestScore) bestIdx.push_back(i);
                }
            }

            if (!ctx.timedOut && !bestIdx.empty()) {
                overallBestScore = bestScore;
                overallBestIdx = bestIdx;

                auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - ctx.start).count();

                std::cout << "Completed depth " << depth
                        << ". Best score: " << overallBestScore
                        << ". Candidates: " << bestIdx.size()
                        << ". Time: " << elapsedMs << " ms\n";

                if (std::abs(overallBestScore) >= WIN) break;
            }
        }

        if (overallBestIdx.empty()) {
            std::cerr << "Error: No best move identified after search. Returning first successor.\n";
            return successors[0];
        }

        static std::mt19937_64 rng{std::random_device{}()};
        std::uniform_int_distribution<int> dist(0, overallBestIdx.size() - 1);
        return successors[overallBestIdx[dist(rng)]];
    }
};
