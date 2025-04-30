#include "AtariGo.h"
#include "Masks.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <utility>
#include <queue>

#include "BBUtils.h"


Bitboard128 AtariGo::getNeighbourBits(Bitboard128 bitboard) {
    // North: shift up BOARD_EDGE rows, then clamp off any bits > BOARD_SIZE-1
    const Bitboard128 north = (bitboard << BOARD_EDGE) & FULL_BOARD_MASK;

    // South: shift down BOARD_EDGE rows (shifting in zeros on top)
    const Bitboard128 south = bitboard >> BOARD_EDGE;

    // East/West: mask *before* shifting to prevent wrap-around
    const Bitboard128 east  = (bitboard & ~LAST_COLUMN_MASK)  << 1;
    const Bitboard128 west  = (bitboard & ~FIRST_COLUMN_MASK) >> 1;

    // Combine
    return north | south | east | west;
}

std::vector<Board> AtariGo::generateSuccessors(const Board& state) {
    std::vector<std::pair<int, Board>> scored;
    scored.reserve(BOARD_SIZE);

    // Special-case: first move in center
    if (state.getTurn() == 1) {
        Board center = state;
        center.setStone(BOARD_EDGE / 2, BOARD_EDGE / 2);
        return { center };
    }

    const Bitboard128 occupiedBits = state.getBlackBits() | state.getWhiteBits();

    const Bitboard128 adjacentBits = getNeighbourBits(occupiedBits);

    Bitboard128 successorBits = adjacentBits & ~occupiedBits;

    while (successorBits) {
        const int pos = BBUtils::popLSB(successorBits);
        Board child = state;
        child.setStone(pos);
        calculateHeuristic(child);
        scored.emplace_back(child.getHeuristic(), child);
    }

    if (state.getPlayerToMove() == BLACK) {
        std::sort(scored.begin(), scored.end(),
                  [](auto &a, auto &b) { return a.first < b.first; });
    } else {
        std::sort(scored.begin(), scored.end(),
                  [](auto &a, auto &b) { return a.first > b.first; });
    }

    std::vector<Board> successors;
    successors.reserve(scored.size());
    for (auto &p : scored)
        successors.push_back(p.second);

    return successors;
}

void AtariGo::computeLibertiesHeuristic(
    const Board &state,
    int &minBlackLib,        // out: smallest number of liberties in any black group
    int &minWhiteLib,        // out: same for white
    int &countMinBLibGroups, // out: how many black groups hit that minimum
    int &countMinWLibGroups, // out: how many white groups hit that minimum
    int &totalBlackLib,      // out: sum of liberties over all black groups
    int &totalWhiteLib       // out: sum of liberties over all white groups
) {
    // Cache bitboards
    const Bitboard128 blackBitboard = state.getBlackBits();
    const Bitboard128 whiteBitboard = state.getWhiteBits();
    const Bitboard128 occupiedBits = blackBitboard | whiteBitboard;

    // Initialize accumulators
    minBlackLib      = BOARD_SIZE;
    minWhiteLib      = BOARD_SIZE;
    countMinBLibGroups = 0;
    countMinWLibGroups = 0;
    totalBlackLib    = 0;
    totalWhiteLib    = 0;

    // Helper lambda to flood each color’s groups
    auto floodColor = [&](Bitboard128 pool, bool isBlack) {
        // Which bitboard and accumulators to use
        const Bitboard128 stoneBits = isBlack ? blackBitboard : whiteBitboard;
        int &minLib   = isBlack ? minBlackLib   : minWhiteLib;
        int &countMin = isBlack ? countMinBLibGroups : countMinWLibGroups;
        int &totalLib = isBlack ? totalBlackLib : totalWhiteLib;

        // Peel off one connected component at a time
        while (pool) {
            // Start a new group from one seed stone
            const int seed = BBUtils::popLSB(pool);
            Bitboard128 group = static_cast<Bitboard128>(1) << seed;

            // Grow the group by adding any same-color neighbors
            while (true) {
                const Bitboard128 newStones =
                    getNeighbourBits(group)  // all nbrs of current group bits
                    & stoneBits              // only same-color stones
                    & ~group;                // only those not already in group
                if (!newStones) break;
                group |= newStones;
            }

            // Remove entire group from pool so we don’t revisit it
            pool &= ~group;

            // Compute liberties in one shot
            const Bitboard128 libsBB =
                getNeighbourBits(group)     // all neighbors of the group
                & ~occupiedBits;            // only empty squares
            const int libs = BBUtils::bitCount(libsBB);

            // Update totals and minima
            totalLib += libs;
            if (libs < minLib) {
                minLib = libs;
                countMin = 1;
            } else if (libs == minLib) {
                ++countMin;
            }
        }
    };

    // Flood both colors
    floodColor(blackBitboard, true);
    floodColor(whiteBitboard, false);
}


void AtariGo::calculateHeuristic(Board &state) {
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

    const Player nextMove = state.getPlayerToMove();
    const Player thisMove = nextMove == BLACK ? WHITE : BLACK;

    // If one color has no liberties, there is a win
    if (minW == 0 && minB > 0) score = -WIN;
    else if (minB == 0 && minW > 0) score = WIN;
    else if (minB == 0 && minW == 0) score = thisMove == BLACK ? -WIN : WIN;

    else {
        // penalize if one color has only one liberty
        if (minB == 1 && minW > 1) score = ATARI_PENALTY * countB;
        else if (minW == 1 && minB > 1) score = -ATARI_PENALTY * countW;

        // penalize if one color has two liberties
        else if (minB == 2 && minW > 2) score = NEAR_ATARI_PENALTY;
        else if (minW == 2 && minB > 2) score = -NEAR_ATARI_PENALTY;

        // add some weight to the difference in liberties
        score += minB - minW;
        score += totalWhiteLib - totalBlackLib;
    }

    state.setHeuristic(score);
}


bool AtariGo::isTerminal(Board &state) {
    const int score = state.getHeuristic();
    return score >= WIN || score <= -WIN;
}

void AtariGo::print(Board &board) {
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

