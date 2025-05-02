#ifndef ATARIGO_H
#define ATARIGO_H

#include <vector>
#include "Board.h"

class AtariGo {
private:
    static void computeLibertiesHeuristic(const Board& state, int& minBlackLib, int& minWhiteLib, int& countMinBLibGroups, int& countMinWLibGroups, int& totalBlackLib, int& totalWhiteLib) ;
public:
    static void calculateHeuristic(Board& state);
    [[nodiscard]] static std::vector<Board> generateSuccessors(const Board& state) ;
    [[nodiscard]] static bool isTerminal(Board& state);
    static void print(Board& board);
};

#endif
