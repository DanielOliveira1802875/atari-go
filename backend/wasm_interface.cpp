#include <emscripten.h>
#include <cstring>
#include <string>
#include <iostream>
#include "Board.h"
#include "MiniMax.cpp"
#include "Globals.h"

struct BoardParseResult {
    Board board;
    int timeLimit;
    int maxDepth;
};

BoardParseResult parseBoard(const char* boardStr) {
    BoardParseResult result;
    result.timeLimit = 3000; // Default time limit in ms
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
            std::cout << "Failed to parse time limit, using default 1000ms\n";
        }
    }

    // Parse max depth if available
    if (parts.size() > 2) {
        try {
            result.maxDepth = std::stoi(parts[2]);
        } catch (...) {
            std::cout << "Failed to parse max depth, using default MAX_SEARCH_DEPTH\n";
        }
    }

    // Parse the board
    for (int i = 0; i < BOARD_SIZE && i < boardData.length(); i++) {
        const char c = std::tolower(boardData[i]);
        if (c == 'b') result.board.setBlack(i);
        else if (c == 'w') result.board.setWhite(i);
    }
    AtariGo::calculateHeuristic(result.board);

    std::cout << "Parsed time limit: " << result.timeLimit << "ms\n";
    std::cout << "Parsed max depth: " << result.maxDepth << "\n";
    std::cout << "Turn: " << static_cast<int>(result.board.getTurn()) << " " << (result.board.getPlayerToMove() == BLACK ? "BLACK" : "WHITE") << "\n";
    std::cout << "Parsed board state:\n";
    AtariGo::print(result.board);

    return result;
}

std::string boardToString(const Board& board) {
    std::string result;
    result.reserve(BOARD_SIZE);
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
        std::cout << "getBestMove called with board: " << boardStr << "\n";
        auto [board, timeLimit, maxDepth] = parseBoard(boardStr);

        MiniMax minimax;

        std::cout << "Calling minimax with time limit of " << timeLimit << "ms...\n";
        const Board bestMove = minimax.getBestMove(board, std::chrono::milliseconds(timeLimit), maxDepth);
        std::cout << "Minimax returned.\n";

        std::string result = boardToString(bestMove);
        if (bestMove.getHeuristic() == WIN) result += ";WHITE";
        else if (bestMove.getHeuristic() == -WIN) result += ";BLACK";
        else result += ";";
        result += ";" + std::to_string(bestMove.getHeuristic());
        std::cout << "Best move board state: " << result << "\n";

        const auto ret = new char[result.size() + 1];
        std::strcpy(ret, result.c_str());
        return ret;
    }

    EMSCRIPTEN_KEEPALIVE
    const char* checkCapture(const char* boardStr) {
        std::cout << "Checking capturing with board: " << boardStr << "\n";
        BoardParseResult parsedResult = parseBoard(boardStr);
        auto capturedBitBoard = AtariGo::getCapturedGroups(parsedResult.board);
        std::string resultStr = "";
        while (capturedBitBoard) {
            const int pos = popLSB(capturedBitBoard);
            resultStr += std::to_string(pos) + ",";
        }

        if (!resultStr.empty()) resultStr.pop_back(); // Remove the last comma

        if (parsedResult.board.getHeuristic() >= WIN) resultStr += ";WHITE";
        else if (parsedResult.board.getHeuristic() <= -WIN) resultStr += ";BLACK";

        const auto ret = new char[resultStr.size() + 1];
        std::strcpy(ret, resultStr.c_str());
        std::cout << "Captured bitboard: " << resultStr << "\n";
        return ret;
    }


} // extern "C"