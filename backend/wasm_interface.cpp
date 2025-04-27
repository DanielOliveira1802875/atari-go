#include <emscripten.h>
#include <cstring>
#include <string>
#include <iostream>
#include "Board.h"
#include "MiniMax.cpp"
#include "Globals.h"
#include "NeighborMasks.h"
#include "Zobrist.h"

struct BoardParseResult {
    Board board;
    int timeLimit;
    int maxDepth;
};

BoardParseResult parseBoard(const char* boardStr) {
    BoardParseResult result;
    result.timeLimit = 1000; // Default time limit in ms
    result.maxDepth = 64; // Default max depth

    std::string input(boardStr);
    std::vector<std::string> parts;

    // Split input by semicolons
    size_t pos = 0;
    std::string token;
    while ((pos = input.find(';')) != std::string::npos) {
        token = input.substr(0, pos);
        parts.push_back(token);
        input.erase(0, pos + 1);
    }

    if (!input.empty()) {
        parts.push_back(input);
    }

    // First part is the board
    std::string boardData = parts[0];

    // Parse time limit if available
    if (parts.size() > 1) {
        try {
            result.timeLimit = std::stoi(parts[1]);
        } catch (...) {
            std::cerr << "Failed to parse time limit, using default 1000ms\n";
        }
    }

    // Parse max depth if available
    if (parts.size() > 2) {
        try {
            result.maxDepth = std::stoi(parts[2]);
        } catch (...) {
            std::cerr << "Failed to parse max depth, using default MAX_SEARCH_DEPTH\n";
        }
    }

    // Parse the board
    for (int i = 0; i < BOARD_SIZE && i < boardData.length(); i++) {
        char c = std::tolower(boardData[i]);
        if (c == 'b') result.board.setBlack(i);
        else if (c == 'w') result.board.setWhite(i);
    }

    return result;
}

std::string boardToString(const Board& board) {
    std::string result;
    result.reserve(81);
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (board.isBlack(i)) result += 'B';
        else if (board.isWhite(i)) result += 'W';
        else result += '.';
    }

    return result;
}

extern "C" {

    EMSCRIPTEN_KEEPALIVE
    const char* getBestMove(const char* boardStr) {
        std::cerr << "Parsing board: " << boardStr << "\n";
        BoardParseResult parseResult = parseBoard(boardStr);
        std::cerr << "Board parsed. Player to play: "
            << (parseResult.board.getPlayerToMove() == BLACK ? "BLACK" : "WHITE")
            << " with time limit: " << parseResult.timeLimit << "ms\n";

        initZobrist();
        initNeighborMasks();
        MiniMax minimax;

        std::cerr << "Calling minimax with time limit of " << parseResult.timeLimit << "ms...\n";
        Board bestMove = minimax.getBestMove(parseResult.board, std::chrono::milliseconds(parseResult.timeLimit), parseResult.maxDepth);
        std::cerr << "Minimax returned.\n";

        std::string result = boardToString(bestMove);
        if (bestMove.getHeuristic() == WIN) result += ";WHITE";
        else if (bestMove.getHeuristic() == -WIN) result += ";BLACK";
        std::cerr << "Best move board state: " << result << "\n";

        char* ret = new char[result.size() + 1];
        std::strcpy(ret, result.c_str());
        return ret;
    }

} // extern "C"