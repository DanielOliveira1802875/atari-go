#include "AtariGo.h"
#include <iostream>
#include <algorithm>
#include <utility>
#include <queue>

#include "BBUtils.h"


std::vector<Board> AtariGo::generateSuccessors(const Board &state) {
    std::vector<std::pair<int, Board> > scored;
    scored.reserve(BOARD_SIZE);

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
    int &minBlackLib1, // out: smallest amount liberties in any black group
    int &minWhiteLib1, // out: smallest amount liberties in any white group
    int &minBlackLib2, // out: second-smallest amount liberties in any black group
    int &minWhiteLib2, // out: second-smallest amount liberties in any white group
    int &minBlackLib3, // out: third-smallest amount liberties in any white group
    int &minWhiteLib3, // out: third-smallest amount liberties in any white group
    int &minBlackLib4, // out: fourth-smallest amount liberties in any black group
    int &minWhiteLib4, // out: fourth-smallest amount liberties in any white group
    int &countMinBLibGroups, // out: how many black groups hit that minimum
    int &countMinWLibGroups, // out: how many white groups hit that minimum
    int &totalBlackLib, // out: total amount liberties for black
    int &totalWhiteLib // out: total amount liberties for white

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
        int &totalExclusiveLib = isBlack ? totalBlackLib : totalWhiteLib;
        Bitboard128 uniqueLibsBB = 0;

        // Peel off one connected component at a time
        while (pool) {
            // Start a new group from one seed stone
            const Bitboard128 seed = static_cast<Bitboard128>(1) << popLSB(pool);
            const Bitboard128 group = floodFillGroupBits(stoneBits, seed);

            // Remove entire group from pool so we don’t revisit it
            pool &= ~group;

            // Compute liberties
            const Bitboard128 libsBB = getLibertyBits(group, occupiedBits);
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
    computeLibertiesHeuristic( state,
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
                              totalWhiteLib);

    int score = 0;

    const Player nextMove = state.getPlayerToMove();
    const Player thisMove = nextMove == BLACK ? WHITE : BLACK;

    // If one color has no liberties, there is a win
    if (minW == 0 && minB > 0) score = -WIN;
    else if (minB == 0 && minW > 0) score = WIN;
    else if (minB == 0 && minW == 0) score = thisMove == BLACK ? -WIN : WIN;

    else {
        score += (minW  - minB ) * 10'000;
        score += (minW2 - minB2) * 1'000;
        score += (minW3 - minB3) * 100;
        score += (minW4 - minB4) * 10;
        score += totalWhiteLib - totalBlackLib;

    }

    state.setHeuristic(score);
}

Bitboard128 AtariGo::getCapturedGroups(Board &state) {

    calculateHeuristic(state);
    const int heuristic = state.getHeuristic();
    if (heuristic > -WIN && heuristic < WIN) return 0;

    const Bitboard128 capturedBitboard = heuristic >= WIN
        ? state.getBlackBits()
        : state.getWhiteBits();

    Bitboard128 pool = capturedBitboard;

    const Bitboard128 occupied = state.getBlackBits() | state.getWhiteBits();
    Bitboard128 capturedMask = 0;

    while (pool) {
        const int pos = popLSB(pool);
        const Bitboard128 seed = static_cast<Bitboard128>(1) << pos;

        // flood-fill its entire group
        const Bitboard128 group = floodFillGroupBits( capturedBitboard, seed);

        // remove from pool
        pool &= ~group;

        // compute its liberties
        const Bitboard128 libs = getLibertyBits(group, occupied);
        if (bitCount(libs) == 0) {
            // it's captured, accumulate it
            capturedMask |= group;
        }
    }

    return capturedMask;
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
