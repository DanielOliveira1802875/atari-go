#ifndef ATARIGO_H
#define ATARIGO_H

#include <vector>
#include "Board.h"

class AtariGo
{
private:
    static void computeLiberties(
        const Board &state,
        int &minBlackLib1, int &minWhiteLib1,
        int &countMinB1LibGroups, int &countMinW1LibGroups,
        int &uniqueTotalBlackLib, // Count of unique black liberty points
        int &uniqueTotalWhiteLib  // Count of unique white liberty points
    );

public:
    static void computeHeuristic(Board &state);

    [[nodiscard]] static std::vector<Board> generateSuccessors(const Board &state);

    [[nodiscard]] static bool isTerminal(const Board &state);

    // If state is terminal, returns the bitboard of the captured group; otherwise returns 0.
    [[nodiscard]] static Bitboard128 getCapturedGroups(Board &state);

    // Returns true if the move was suicidal.
    [[nodiscard]] static bool wasMoveSuicidal(const Board &state);

    static void print(Board &board);
};

#endif
