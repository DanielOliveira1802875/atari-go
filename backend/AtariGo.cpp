#include "AtariGo.h"
#include <iostream>
#include <algorithm>
#include <utility>
#include <queue>

#include "BBUtils.h"

std::vector<Board> AtariGo::generateSuccessors(const Board &state) {
    static std::vector<std::pair<int, Board> > scored_successors(BOARD_SIZE);
    scored_successors.clear();

    const Bitboard128 occupiedBits = state.getBlackBits() | state.getWhiteBits();
    Bitboard128 successorBits = getNeighbourBits(occupiedBits);

    while (successorBits) {
        const int pos = popLSB(successorBits);

        scored_successors.emplace_back(0, state);
        auto &[score, successor] = scored_successors.back();

        successor.setStone(pos);
        calculateHeuristic(successor);
        score = successor.getHeuristic();
    }

    if (state.getPlayerToMove() == BLACK) {
        std::sort(scored_successors.begin(), scored_successors.end(),
                  [](const auto &a, const auto &b) { return a.first < b.first; });
    } else {
        std::sort(scored_successors.begin(), scored_successors.end(),
                  [](const auto &a, const auto &b) { return a.first > b.first; });
    }

    std::vector<Board> successors;
    successors.reserve(scored_successors.size());

    for (auto &[score_val, board_obj]: scored_successors) {
        successors.push_back(std::move(board_obj));
    }
    return successors;
}

void AtariGo::computeLibertiesHeuristic(
    const Board &state,
    int &minBlackLib1, int &minWhiteLib1,
    int &minBlackLib2, int &minWhiteLib2,
    int &minBlackLib3, int &minWhiteLib3,
    int &minBlackLib4, int &minWhiteLib4,
    int &countMinB1LibGroups, int &countMinW1LibGroups,
    int &uniqueTotalBlackLib,
    int &uniqueTotalWhiteLib) {
    // Cache bitboards
    const Bitboard128 blackBitboard = state.getBlackBits();
    const Bitboard128 whiteBitboard = state.getWhiteBits();
    const Bitboard128 occupiedBits = blackBitboard | whiteBitboard;

    // Initialize output parameters that are summed or counted
    minBlackLib1 = minBlackLib2 = minBlackLib3 = minBlackLib4 = minWhiteLib1 = minWhiteLib2 = minWhiteLib3 = minWhiteLib4 = state.getTurn() < 50 ? STARTING_MIN_LIBERTIES : BOARD_SIZE;
    countMinB1LibGroups = 0;
    countMinW1LibGroups = 0;
    uniqueTotalBlackLib = 0;
    uniqueTotalWhiteLib = 0;

    // Helper lambda to flood each color’s groups
    auto floodColor = [&](Bitboard128 pool, const bool isBlack) {
        // Which bitboard and accumulators to use
        const Bitboard128 stoneBits = isBlack ? blackBitboard : whiteBitboard;
        int &minLib1 = isBlack ? minBlackLib1 : minWhiteLib1;
        int &minLib2 = isBlack ? minBlackLib2 : minWhiteLib2;
        int &minLib3 = isBlack ? minBlackLib3 : minWhiteLib3;
        int &minLib4 = isBlack ? minBlackLib4 : minWhiteLib4;
        int &countMin1 = isBlack ? countMinB1LibGroups : countMinW1LibGroups;

        Bitboard128 allUniqueLibsBBForColor = 0;

        while (pool) {
            const Bitboard128 seed = static_cast<Bitboard128>(1) << popLSB(pool);
            const Bitboard128 group = floodFillGroupBits(stoneBits, seed);

            // Remove entire group from pool so we don’t revisit it
            pool &= ~group;

            // Compute liberties
            const Bitboard128 libsBB = getLibertyBits(group, occupiedBits);
            const int libs = bitCount(libsBB);

            // If no stones, libs will be 0, skip if group is empty (should not happen if pool comes from stoneBits)
            if (bitCount(group) == 0)
                continue;

            allUniqueLibsBBForColor |= libsBB; // Accumulate unique liberties for this color

            if (libs < minLib1) {
                minLib4 = minLib3;
                minLib3 = minLib2;
                minLib2 = minLib1;
                minLib1 = libs;
                countMin1 = 1;
            } else if (libs == minLib1) {
                countMin1++;
                if (libs < minLib2) {
                    minLib4 = minLib3;
                    minLib3 = minLib2;
                    minLib2 = libs;
                } else if (libs < minLib3) {
                    minLib4 = minLib3;
                    minLib3 = libs;
                } else if (libs < minLib4) {
                    minLib4 = libs;
                }
            } else if (libs < minLib2) {
                minLib4 = minLib3;
                minLib3 = minLib2;
                minLib2 = libs;
            } else if (libs < minLib3) {
                minLib4 = minLib3;
                minLib3 = libs;
            } else if (libs < minLib4) {
                minLib4 = libs;
            }
        }
        if (isBlack) {
            uniqueTotalBlackLib = bitCount(allUniqueLibsBBForColor);
        } else {
            uniqueTotalWhiteLib = bitCount(allUniqueLibsBBForColor);
        }
    };

    // Flood both colors
    floodColor(blackBitboard, true);
    floodColor(whiteBitboard, false);
}

void AtariGo::calculateHeuristic(Board &state) {
    if (state.getIsHeuristicCalculated())
        return;

    int minB1, minW1, minB2, minW2, minB3, minW3, minB4, minW4;
    int countMinB1Groups, countMinW1Groups;
    int uniqueTotalBlackLib, uniqueTotalWhiteLib;

    computeLibertiesHeuristic(
        state,
        minB1, minW1, minB2, minW2, minB3, minW3, minB4, minW4,
        countMinB1Groups, countMinW1Groups,
        uniqueTotalBlackLib, uniqueTotalWhiteLib);

    int score = 0;

    const Player nextPlayer = state.getPlayerToMove();
    const Player currentPlayer = (nextPlayer == BLACK) ? WHITE : BLACK;

    const bool blackGroupNowHasZeroLiberties = minB1 == 0;
    const bool whiteGroupNowHasZeroLiberties = minW1 == 0;

    // Current player captured opponent's stones.
    if (currentPlayer == WHITE) {
        if (blackGroupNowHasZeroLiberties) {
            score = WIN;
            state.setHeuristic(score);
            return;
        }
        if (whiteGroupNowHasZeroLiberties) {
            score = -WIN;
            state.setHeuristic(score);
            return;
        }
    } else {
        // currentPlayer == BLACK
        if (whiteGroupNowHasZeroLiberties) {
            score = -WIN;
            state.setHeuristic(score);
            return;
        }
        if (blackGroupNowHasZeroLiberties) {
            score = WIN;
            state.setHeuristic(score);
            return;
        }
    }
    if (currentPlayer == BLACK) {
        if (minB1 == 1) {
            // Black in atari; White will capture next.
            score += ATARI_THREAT_SCORE * (countMinB1Groups);
        } else if (minW1 == 1) {
            // White in atari.
            score -= ATARI_THREAT_SCORE * (countMinW1Groups);
        }
    } else {
        // currentPlayer == WHITE
        if (minW1 == 1) {
            // White in atari; Black will capture next.
            score -= ATARI_THREAT_SCORE * (countMinW1Groups);
        } else if (minB1 == 1) {
            // Black in atari.
            score += ATARI_THREAT_SCORE * (countMinB1Groups);
        }
    }

    if (minW1 > 1 && minB1 > 1) {
        // Both groups have more than 1 liberty, evaluate based on liberties.
        score += (minW1 - minB1) * MIN_LIB_1_MULTIPLIER;
    }

    score += (minW2 - minB2) * MIN_LIB_2_MULTIPLIER;
    score += (minW3 - minB3) * MIN_LIB_3_MULTIPLIER;
    score += (minW4 - minB4) * MIN_LIB_4_MULTIPLIER;

    score += (uniqueTotalWhiteLib - uniqueTotalBlackLib) * UNIQUE_LIB_MULTIPLIER;

    state.setHeuristic(score);
}

Bitboard128 AtariGo::getCapturedGroups(Board &state) {
    calculateHeuristic(state);
    const int heuristic = state.getHeuristic();
    if (heuristic > -WIN && heuristic < WIN)
        return 0;

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
        const Bitboard128 group = floodFillGroupBits(capturedBitboard, seed);

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
            char c = (s == Black)
                         ? 'B'
                         : (s == White)
                               ? 'W'
                               : '.';
            std::cout << c << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\nHeuristic: " << board.getHeuristic() << std::endl;
}
