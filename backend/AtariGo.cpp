#include "AtariGo.h"
#include <iostream>
#include <algorithm>
#include <complex>
#include <utility>
#include <queue>

#include "BBUtils.h"

int AtariGo::removeRandomSuccessorsPercentage = 0;

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
        computeHeuristic(successor);
        score = successor.getHeuristic();
    }

    if (removeRandomSuccessorsPercentage > 0 && !scored_successors.empty()) {
        // Remove a percentage of successors randomly
        const size_t initialSize = scored_successors.size();
        size_t toRemove = static_cast<size_t>(initialSize * (removeRandomSuccessorsPercentage / 100.0));

        // Ensure at least one successor remains
        if (toRemove >= initialSize) toRemove = initialSize - 1;

        if (toRemove > 0) {
            std::shuffle(scored_successors.begin(), scored_successors.end(), getRandom());
            scored_successors.resize(initialSize - toRemove);
        }
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

void AtariGo::computeLiberties(
    const Board &state,
    int &minBlackLib1, int &minWhiteLib1,
    int &countMinB1LibGroups, int &countMinW1LibGroups,
    int &uniqueTotalBlackLib,
    int &uniqueTotalWhiteLib) {
    // Cache bitboards
    const Bitboard128 blackBitboard = state.getBlackBits();
    const Bitboard128 whiteBitboard = state.getWhiteBits();
    const Bitboard128 occupiedBits = blackBitboard | whiteBitboard;

    // Initialize output parameters that are summed or counted
    minBlackLib1 = minWhiteLib1 = settings.STARTING_MIN_LIBERTIES;
    countMinB1LibGroups = 0;
    countMinW1LibGroups = 0;
    uniqueTotalBlackLib = 0;
    uniqueTotalWhiteLib = 0;

    // Helper lambda to flood each color’s groups
    auto floodColor = [&](Bitboard128 pool, const bool isBlack) {
        // Which bitboard and accumulators to use
        const Bitboard128 stoneBits = isBlack ? blackBitboard : whiteBitboard;
        int &minLib1 = isBlack ? minBlackLib1 : minWhiteLib1;
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
                minLib1 = libs;
                countMin1 = 1;
            } else if (libs == minLib1) {
                countMin1++;
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

void AtariGo::computeHeuristic(Board &state) {
    if (state.getIsHeuristicCalculated())
        return;

    int minB1, minW1;
    int countMinB1Groups, countMinW1Groups;
    int uniqueTotalBlackLib, uniqueTotalWhiteLib;

    computeLiberties(
        state,
        minB1, minW1,
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
            score += settings.ATARI_THREAT_MULTIPLIER * (countMinB1Groups);
        } else if (minW1 == 1) {
            // White in atari.
            score -= settings.ATARI_THREAT_MULTIPLIER * (countMinW1Groups);
        }
    } else {
        // currentPlayer == WHITE
        if (minW1 == 1) {
            // White in atari; Black will capture next.
            score -= settings.ATARI_THREAT_MULTIPLIER * (countMinW1Groups);
        } else if (minB1 == 1) {
            // Black in atari.
            score += settings.ATARI_THREAT_MULTIPLIER * (countMinB1Groups);
        }
    }

    if (minW1 > 1 && minB1 > 1) {
        // Both groups have more than 1 liberty, evaluate based on liberties.
        score += (minW1 - minB1) * settings.MIN_LIB_MULTIPLIER;
    }

    score += (uniqueTotalWhiteLib - uniqueTotalBlackLib) * settings.UNIQUE_LIB_MULTIPLIER;

    state.setHeuristic(score);
}

Bitboard128 AtariGo::getCapturedGroups(Board &state) {
    computeHeuristic(state);
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

bool AtariGo::wasMoveSuicidal(const Board &state) {

    const int heuristic = state.getHeuristic();

    // If it isn't a WIN or LOSS, then the move wasn't a suicide.
    if (heuristic != WIN && heuristic != -WIN) return false;


    // Determine who made the last move.
    const Player nextPlayerToMove = state.getPlayerToMove();
    const Player playerWhoMoved = (nextPlayerToMove == BLACK) ? WHITE : BLACK;

    if (playerWhoMoved == WHITE) {
        // If White moved and Black won, it was a suicide by White.
        return heuristic == -WIN;
    } else { // playerWhoMoved == BLACK
        // If Black moved and White won, it was a suicide by Black.
        return heuristic == WIN;
    }
}
