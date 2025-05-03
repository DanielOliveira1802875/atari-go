#include "AtariGo.h"
#include "Masks.h"
#include <iostream>
#include <algorithm>
#include <utility>
#include <queue>

#include "BBUtils.h"


std::vector<Board> AtariGo::generateSuccessors(const Board &state) {
    std::vector<std::pair<int, Board> > scored;
    scored.reserve(BOARD_SIZE);

    // Special-case: first move in center
    if (state.getTurn() == 1) {
        Board center = state;
        center.setStone(BOARD_EDGE / 2, BOARD_EDGE / 2);
        calculateHeuristic(center);
        return {center};
    }

    const Bitboard128 occupiedBits = state.getBlackBits() | state.getWhiteBits();

    Bitboard128 successorBits = getNeighbourBits(occupiedBits);


    while (successorBits) {
        const int pos = popLSB(successorBits);
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
    for (auto &p: scored)
        successors.push_back(p.second);
    return successors;
}

void AtariGo::computeLibertiesHeuristic(
    const Board &state,
    int &minBlackLib1, // out: smallest number of liberties in any black group
    int &minWhiteLib1, // out: smallest number of liberties in any white group
    int &minBlackLib2, // out: second-smallest number of liberties in any black group
    int &minWhiteLib2, // out: second-smallest number of liberties in any white group
    int &minBlackLib3, // out: third-smallest number of liberties in any white group
    int &minWhiteLib3, // out: third-smallest number of liberties in any white group
    int &minBlackLib4, // out: fourth-smallest number of liberties in any black group
    int &minWhiteLib4, // out: fourth-smallest number of liberties in any white group
    int &countMinBLibGroups, // out: how many black groups hit that minimum
    int &countMinWLibGroups, // out: how many white groups hit that minimum
    int &totalBlackLib, // out: sum of liberties over all black groups
    int &totalWhiteLib, // out: sum of liberties over all white groups
    int &totalExclusiveBlackLib,
    int &totalExclusiveWhiteLib

) {
    // Cache bitboards
    const Bitboard128 blackBitboard = state.getBlackBits();
    const Bitboard128 whiteBitboard = state.getWhiteBits();
    const Bitboard128 occupiedBits = blackBitboard | whiteBitboard;
    // Helper lambda to flood each color’s groups
    auto floodColor = [&](Bitboard128 pool, const bool isBlack) {
        // Which bitboard and accumulators to use
        const Bitboard128 stoneBits = isBlack ? blackBitboard : whiteBitboard;
        int &minLib = isBlack ? minBlackLib1 : minWhiteLib1;
        int &minLib2 = isBlack ? minBlackLib2 : minWhiteLib2;
        int &minLib3 = isBlack ? minBlackLib3 : minWhiteLib3;
        int &minLib4 = isBlack ? minBlackLib4 : minWhiteLib4;
        int &countMin = isBlack ? countMinBLibGroups : countMinWLibGroups;
        int &totalLib = isBlack ? totalBlackLib : totalWhiteLib;
        int &totalExclusiveLib = isBlack ? totalExclusiveBlackLib : totalExclusiveWhiteLib;
        Bitboard128 uniqueLibsBB = 0;

        // Peel off one connected component at a time
        while (pool) {
            // Start a new group from one seed stone
            const int seed = popLSB(pool);
            Bitboard128 group = static_cast<Bitboard128>(1) << seed;

            // Grow the group by adding any same-color neighbors
            while (true) {
                const Bitboard128 newStones =
                        getNeighbourBits(group) // all nbrs of current group bits
                        & stoneBits; // only same-color stones
                if (!newStones) break;
                group |= newStones;
            }

            // Remove entire group from pool so we don’t revisit it
            pool &= ~group;

            // Compute liberties
            const Bitboard128 libsBB =
                    getNeighbourBits(group) // all neighbors of the group
                    & ~occupiedBits; // only empty squares
            const int libs = bitCount(libsBB);

            // Update totals and minima
            uniqueLibsBB |= libsBB;

            if (libs == minLib) ++countMin;

            if (libs <= minLib) {
                // shift everything down
                minLib4 = minLib3;
                minLib3 = minLib2;
                minLib2 = minLib;
                minLib  = libs;
                countMin = 1;            // new absolute minimum
            }
            else if (libs <= minLib2) {
                // new 2nd-smallest
                minLib4 = minLib3;
                minLib3 = minLib2;
                minLib2 = libs;
            }
            else if (libs <= minLib3) {
                // new 3rd-smallest
                minLib4 = minLib3;
                minLib3 = libs;
            }
            else if (libs < minLib4) {
                // new 4th-smallest
                minLib4 = libs;
            }
            totalLib += libs;
        }
        totalExclusiveLib = bitCount(uniqueLibsBB);
    };

    // Flood both colors
    floodColor(blackBitboard, true);
    floodColor(whiteBitboard, false);
}


void AtariGo::calculateHeuristic(Board &state) {
    // Check if heuristic is already calculated to avoid recomputation
    if (state.getIsHeuristicCalculated()) return;

    int minB = 6;
    int minW = 6;
    int minB2 = 6;
    int minW2 = 6;
    int minB3 = 6;
    int minW3 = 6;
    int minB4 = 6;
    int minW4 = 6;
    int countB = 0;
    int countW = 0;
    int totalBlackLib = 0;
    int totalWhiteLib = 0;
    int totalExclusiveBlackLib = 0;
    int totalExclusiveWhiteLib = 0;
    computeLibertiesHeuristic(state,
                              minB,
                              minW,
                              minB2,
                              minW2,
                              minB3,
                              minW3,
                              minB4,
                              minW4,
                              countB,
                              countW,
                              totalBlackLib,
                              totalWhiteLib,
                              totalExclusiveBlackLib,
                              totalExclusiveWhiteLib);

    int score = 0;

    const Player nextMove = state.getPlayerToMove();
    const Player thisMove = nextMove == BLACK ? WHITE : BLACK;

    // If one color has no liberties, there is a win
    if (minW == 0 && minB > 0) score = -WIN;
    else if (minB == 0 && minW > 0) score = WIN;
    else if (minB == 0 && minW == 0) score = thisMove == BLACK ? -WIN : WIN;

    else {
        // penalize if one color has only one liberty
        /*if (minB == 1 && minW > 1) {
            score = ATARI_PENALTY * countB;
            state.setHeuristic(score);
            return;
        }
        if (minW == 1 && minB > 1) {
            score = -ATARI_PENALTY * countW;
            state.setHeuristic(score);
            return;
        }*/

        /*// penalize if one color has two liberties
        else if (minB == 2 && minW > 2) score = NEAR_ATARI_PENALTY;

        else if (minW == 2 && minB > 2) score = -NEAR_ATARI_PENALTY;*/

        /*if (minW == 2 && minB == 2) {
            score += (totalWhiteLib - totalBlackLib) * 1000;
        }*/

        // add some weight to the difference in liberties
        score += (minW  - minB ) * 10'000;
        score += (minW2 - minB2) * 1000;
        score += (minW3 - minB3) * 100;
        score += (minW4 - minB4) * 10;
        score += totalExclusiveWhiteLib - totalExclusiveBlackLib;

    }

    state.setHeuristic(score);
}


bool AtariGo::isTerminal(const Board &state) {
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
