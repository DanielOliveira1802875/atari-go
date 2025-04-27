#include "AtariGo.h"
#include "NeighborMasks.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <utility>
#include <queue>


//Check if a given square has any occupied orthogonal neighbors.
inline bool hasOccupiedNeighbor(const Board& b, int pos)
{
    // Combine black and white occupancy into a single bitboard
    Bitboard128 occupancy = b.getBlackBits() | b.getWhiteBits();

    // Gets the precomputed mask of orthogonal neighbors for `pos`
    Bitboard128 neighborMask = ORTH_MASK[pos];

    // AND the neighbor mask with the occupancy bitboard:
    // any set bit in common means an occupied neighbor.
    return (neighborMask & occupancy) != 0;
}

std::vector<Board> AtariGo::generateSuccessors(const Board& state) {
    std::vector<std::pair<int, Board>> scoredSuccessors;
    std::vector<Board> successors;
	successors.reserve(BOARD_SIZE);

    // If it's the first turn, place a stone in the center of the board.
    if (state.getTurn() == 1) {
        Board successor = state;
        const unsigned char randX = 4 + rand() % 2;
        const unsigned char randY = 4 + rand() % 2;
        successor.setStone(randX, randY);
        successors.push_back(successor);
        return successors;
    }

    // Generate successors by placing stones in empty positions with occupied neighbors.
    for (int pos = 0; pos < BOARD_SIZE; ++pos) {
        if (!state.isEmpty(pos) || !hasOccupiedNeighbor(state, pos)) continue;

        Board successor = state;
        successor.setStone(pos);

        calculateHeuristic(successor);
		int score = successor.getHeuristic();
        scoredSuccessors.emplace_back(score, successor);
    }

    if (state.getPlayerToMove() == BLACK) { // Black wants to MINIMIZE the score, so sort in ascending order.
        std::sort(scoredSuccessors.begin(), scoredSuccessors.end(),
            [](const std::pair<int, Board>& a, const std::pair<int, Board>& b) {
                return a.first < b.first;
            });
    }
    else { // White wants to MAXIMIZE the score, so sort in descending order.
        std::sort(scoredSuccessors.begin(), scoredSuccessors.end(),
            [](const std::pair<int, Board>& a, const std::pair<int, Board>& b) {
                return a.first > b.first;
            });
    }

    // Extract the sorted boards from the pairs.
	for (const auto& pair : scoredSuccessors) successors.push_back(pair.second);

    return successors;
}

void AtariGo::computeLibertiesHeuristic(
        Board& state,
        int& minBlackLib, int& minWhiteLib,
        int& cntMinB,     int& cntMinW,
        int& totalB,      int& totalW )
{

    Bitboard128 blackBoard = state.getBlackBits();
    Bitboard128 whiteBoard = state.getWhiteBits();
    const Bitboard128 occupationBoard   = blackBoard | whiteBoard;

    // buffer for flood-fill frontier
    int stack[BOARD_SIZE];

    auto pop_lsb = [](Bitboard128& bb) -> int {
        int idx = __builtin_ctzll( static_cast<uint64_t>(bb) );
        if constexpr (sizeof(Bitboard128) > 8)
            idx = (bb & ((static_cast<Bitboard128>(1) << 64) - 1)) ? idx
                 : 64 + __builtin_ctzll(static_cast<uint64_t>(bb >> 64));
        bb &= bb - 1;
        return idx;
    };

    // process both colours
    auto flood = [&](Bitboard128& pool, bool isBlack)
    {
        bool  visited[BOARD_SIZE] = {};
        while (pool)                             // unvisited stones remain
        {
            int seed = pop_lsb(pool);
            if (visited[seed]) continue;

            unsigned char liberties = 0;
            int  stackSize = 0;
            stack[stackSize++] = seed;
            visited[seed] = true;

            // flood-fill this group
            while (stackSize)
            {
                int pos = stack[--stackSize];
                Bitboard128 nbrMask = ORTH_MASK[pos];

                // count liberties on empty neighbours
                Bitboard128 libBits = nbrMask & ~occupationBoard;
                liberties  += __builtin_popcountll( static_cast<uint64_t>(libBits) )
                            + __builtin_popcountll( static_cast<uint64_t>(libBits >> 64) );

                // push same-colour neighbours that aren’t visited yet
                Bitboard128 same = nbrMask & (isBlack ? state.getBlackBits() : state.getWhiteBits());
                while (same)
                {
                    int n = pop_lsb(same);
                    if (!visited[n])
                    {
                        visited[n] = true;
                        stack[stackSize++] = n;
                    }
                }
            }

            // update global stats
            if (isBlack) {
                totalB += liberties;
                if (liberties < minBlackLib) { minBlackLib = liberties; cntMinB = 1; }
                else if (liberties == minBlackLib) ++cntMinB;
            } else {
                totalW += liberties;
                if (liberties < minWhiteLib) { minWhiteLib = liberties; cntMinW = 1; }
                else if (liberties == minWhiteLib) ++cntMinW;
            }
        }
    };

    flood(blackBoard, true);
    flood(whiteBoard, false);
}

void AtariGo::calculateHeuristic(Board& state) {
    // Check if heuristic is already calculated to avoid recomputation
    if (state.getIsHeuristicCalculated()) return;

    int minB = BOARD_SIZE;
    int minW = BOARD_SIZE;
    int countB = 0;
    int countW = 0;
    int totalBlackLib = 0;
    int totalWhiteLib = 0;
    computeLibertiesHeuristic(state, minB, minW, countB, countW, totalBlackLib, totalWhiteLib);

    int score = 0;

    Player nextMove = state.getPlayerToMove();
    Player thisMove = nextMove == BLACK ? WHITE : BLACK;

    // If one color has no liberties, there is a win
    if (minW == 0 && minB > 0) score = -WIN;
    else if (minB == 0 && minW > 0)  score = WIN;
    else if (minB == 0 && minW == 0) score = thisMove == BLACK ? -WIN : WIN;

    else {
        // penalize if one color has only one liberty
        if (minB == 1 && minW > 1 ) score =  ATARI_PENALTY * countB;
        else if (minW == 1 && minB > 1 ) score =  -ATARI_PENALTY * countW;

        // penalize if one color has two liberties
        else if (minB == 2 && minW > 2) score =  NEAR_ATARI_PENALTY;
        else if (minW == 2 && minB > 2) score = -NEAR_ATARI_PENALTY;

        // add some weight to the difference in liberties
        score += minB - minW;
        score += totalWhiteLib - totalBlackLib;
    }

    state.setHeuristic(score);
}



bool AtariGo::isTerminal(Board& state) {
    const int score = state.getHeuristic();
    return score >= WIN || score <= -WIN;
}

void AtariGo::print(Board& board) {
    std::cout << "  ";
    for (int col = 0; col < BOARD_EDGE; ++col) {
        std::cout << (col + 1) << " ";
    }
    std::cout << "\n";

    for (int row = 0; row < BOARD_EDGE; ++row) {
        std::cout << static_cast<char>('A' + row) << " "; // Row label.
        for (int col = 0; col < BOARD_EDGE; ++col) {
            Stone s = board.getStone(row, col);
            char c = (s == Black) ? 'B' : (s == White) ? 'W' : '.';
            std::cout << c << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\nHeuristic: " << board.getHeuristic() << std::endl;
}

