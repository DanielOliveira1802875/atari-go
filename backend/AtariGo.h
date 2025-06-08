#ifndef ATARIGO_H
#define ATARIGO_H

#include <vector>
#include "Board.h"
#include "unordered_dense.h"

class AtariGo
{
private:
    static void computeLibertiesHeuristic(
        const Board &state,
        int &minBlackLib1, int &minWhiteLib1,
        int &minBlackLib2, int &minWhiteLib2,
        int &minBlackLib3, int &minWhiteLib3, // Kept for potential deeper evaluation
        int &minBlackLib4, int &minWhiteLib4, // Kept for potential deeper evaluation
        int &countMinB1LibGroups, int &countMinW1LibGroups,
        int &uniqueTotalBlackLib, // Count of unique black liberty points
        int &uniqueTotalWhiteLib  // Count of unique white liberty points
    );

public:
    static void calculateHeuristic(Board &state);

    [[nodiscard]] static std::vector<Board> generateSuccessors(const Board &state);

    [[nodiscard]] static bool isTerminal(const Board &state);

    // If state is terminal, returns the bitboard of the captured group; otherwise returns 0.
    [[nodiscard]] static Bitboard128 getCapturedGroups(Board &state);

    static void print(Board &board);
};

#endif
