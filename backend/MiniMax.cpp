#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include "unordered_dense.h"
#include "AtariGo.h"

inline Player opponent(Player p) { return p == BLACK ? WHITE : BLACK; }
constexpr int INF = WIN * 2;
constexpr uint64_t CHECK_INTERVAL = 10'000;

inline int distanceAwareScore(const int raw, const int ply) {
    if (raw >= WIN) return raw - ply; // sooner win  -> bigger value
    if (raw <= -WIN) return raw + ply; // later loss -> less negative
    return raw; // heuristic, leave untouched
}



class MiniMax {
    // Transposition table: signature -> (score, depth, bound)
    mutable ankerl::unordered_dense::map<uint64_t, std::tuple<int, int, Bound>> transpositionTable;

    struct SearchContext {
        const std::chrono::steady_clock::time_point start;
        const std::chrono::milliseconds timeLimit;
        bool timedOut = false;
        uint64_t nodeCount = 0;
    };

    int minimax(const Board &state, int depth, int alpha, int beta, SearchContext &ctx, const int ply) const {

        if (ctx.timedOut) return 0;


        if (++ctx.nodeCount % CHECK_INTERVAL == 0) {
            if (std::chrono::steady_clock::now() - ctx.start >= ctx.timeLimit) {
                ctx.timedOut = true;
                return 0;
            }
        }

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
                return score;
            }
        }


        if (depth == 0 || AtariGo::isTerminal(state)) {
            return distanceAwareScore(state.getHeuristic(), ply);
        }

        // Generate successors
        const auto successors = AtariGo::generateSuccessors(state);
        if (successors.empty()) return distanceAwareScore(state.getHeuristic(), ply);

        const Player toMove = state.getPlayerToMove();
        const int origAlpha = alpha;
        const int origBeta = beta;

        // Iterate through successors
        for (const auto &child: successors) {
            int score = minimax(child, depth - 1, alpha, beta, ctx, ply + 1);
            if (ctx.timedOut) return 0;

            if (toMove == WHITE) {
                alpha = std::max(alpha, score);
            } else {
                beta = std::min(beta, score);
            }
            if (alpha >= beta) break;
        }

        int best = toMove == WHITE ? alpha : beta;

        Bound flag = EXACT;
        if (best <= origAlpha) flag = UPPER;
        else if (best >= origBeta) flag = LOWER;
        transpositionTable[signature] = {best, depth, flag};

        return best;
    }

public:
    Board getBestMove(const Board &state, const std::chrono::milliseconds timeLimit, const int depthLimit) {
        static std::mt19937_64 rng{std::random_device{}()};
        SearchContext ctx{std::chrono::steady_clock::now(), timeLimit};
        std::vector<Board> successors = AtariGo::generateSuccessors(state);

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
                AtariGo::calculateHeuristic(successor);
                strongMoves.push_back(successor);
            }
        }
        if (!strongMoves.empty()) {
            std::uniform_int_distribution<int> dist(0, strongMoves.size() - 1);
            successors.push_back(strongMoves[dist(rng)]);
        }
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
        transpositionTable.reserve(10'000'000);
        int overallBestScore = 0;
        std::vector<int> overallBestIdx;
        const Player currentPlayer = state.getPlayerToMove();

        for (int depth = 1; depth <= depthLimit && !ctx.timedOut; ++depth) {
            int bestScore = (currentPlayer == BLACK) ? INF : -INF;
            std::vector<int> bestIdx;

            for (const auto &successor : successors) {
                int score;
                if (currentPlayer == WHITE) {
                    score = minimax(successor, depth - 1, bestScore - 1, INF, ctx, 0);
                    if (score > bestScore) {
                        bestScore = score;
                        bestIdx = {static_cast<int>(&successor - &successors[0])};
                    } else if (score == bestScore) {
                        bestIdx.push_back(static_cast<int>(&successor - &successors[0]));
                    }
                } else {
                    score = minimax(successor, depth - 1, -INF, bestScore + 1, ctx, 0);
                    if (score < bestScore) {
                        bestScore = score;
                        bestIdx = {static_cast<int>(&successor - &successors[0])};
                    } else if (score == bestScore) {
                        bestIdx.push_back(static_cast<int>(&successor - &successors[0]));
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
                        << ". TT Size: " << transpositionTable.size()
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