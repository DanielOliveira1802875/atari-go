#include <iostream>
#include <sstream>
#include <chrono>
#include <random>
#include <string>

#include "Board.h"
#include "AtariGo.h"
#include "Globals.h"
#include "MiniMax.cpp"

Player playAIGame(const Settings &settingsPlayer1, const std::string &player1Name,
                  const Settings &settingsPlayer2, const std::string &player2Name,
                  bool player1PlaysBlack, int thinkingTimeMs, int maxDepth) {
    std::cout << "\n--- New Game ---" << std::endl;
    if (player1PlaysBlack) {
        std::cout << player1Name << " (Black) vs " << player2Name << " (White)" << std::endl;
    } else {
        std::cout << player2Name << " (Black) vs " << player1Name << " (White)" << std::endl;
    }

    Board board;
    MiniMax minimax;

    settings = settingsPlayer1;
    AtariGo::calculateHeuristic(board);

    Player currentPlayerTurn = board.getPlayerToMove();

    while (!AtariGo::isTerminal(board)) {
        std::cout << "\nPlayer to move: " << (currentPlayerTurn == BLACK ? "BLACK" : "WHITE") << std::endl;

        if (currentPlayerTurn == BLACK) {
            settings = player1PlaysBlack ? settingsPlayer1 : settingsPlayer2;
            std::cout << (player1PlaysBlack ? player1Name : player2Name) << " (BLACK) is thinking with its settings..." << std::endl;
        } else {
            settings = !player1PlaysBlack ? settingsPlayer1 : settingsPlayer2;
            std::cout << (!player1PlaysBlack ? player1Name : player2Name) << " (WHITE) is thinking with its settings..." << std::endl;
        }

        board.invalidateHeuristic();
        AtariGo::calculateHeuristic(board);

        AtariGo::print(board);

        auto t0 = std::chrono::high_resolution_clock::now();
        const Board bestMoveBoard = minimax.getBestMove(board, std::chrono::milliseconds(thinkingTimeMs), maxDepth);
        auto t1 = std::chrono::high_resolution_clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        std::cout << "AI thought for " << ms << " ms." << std::endl;

        board = bestMoveBoard;
        currentPlayerTurn = board.getPlayerToMove();
    }

    std::cout << "Game over. Terminal state reached." << std::endl;
    Player winner = NO_PLAYER;
    if (board.getHeuristic() == WIN) {
        winner = WHITE;
    } else if (board.getHeuristic() == -WIN) {
        winner = BLACK;
    }

    AtariGo::print(board);
    return winner;
}


int main() {
Settings settings_v1;
settings_v1.ATARI_THREAT_SCORE = 1'000'000;         // Multiplier for atari threats (default: 1'000'000, 0 disables)
settings_v1.MIN_LIB_1_MULTIPLIER = 2'000;           // Multiplier for the group with the fewest liberties (default: 2'000; 0 disables)
settings_v1.MIN_LIB_2_MULTIPLIER = 200;             // Multiplier for the group with the second fewest liberties (default: 200, 0 disables)
settings_v1.MIN_LIB_3_MULTIPLIER = 100;             // Multiplier for the group with the third fewest liberties (default: 100, 0 disables)
settings_v1.MIN_LIB_4_MULTIPLIER = 40;              // Multiplier for the group with the fourth fewest liberties (default: 40, 0 disables)
settings_v1.UNIQUE_LIB_MULTIPLIER = 20;             // Multiplier for unique liberties (default: 20, 0 disables)
settings_v1.STARTING_MIN_LIBERTIES = 6;             // Starting minimum liberties for each group.

Settings settings_v2;
settings_v2.ATARI_THREAT_SCORE = 1'000'000;         // Multiplier for atari threats (default: 1'000'000, 0 disables)
settings_v2.MIN_LIB_1_MULTIPLIER = 2'000;           // Multiplier for the group with the fewest liberties (default: 2'000; 0 disables)
settings_v2.MIN_LIB_2_MULTIPLIER = 0;               // Multiplier for the group with the second fewest liberties (default: 200, 0 disables)
settings_v2.MIN_LIB_3_MULTIPLIER = 0;               // Multiplier for the group with the third fewest liberties (default: 100, 0 disables)
settings_v2.MIN_LIB_4_MULTIPLIER = 0;               // Multiplier for the group with the fourth fewest liberties (default: 40, 0 disables)
settings_v2.UNIQUE_LIB_MULTIPLIER = 20;             // Multiplier for unique liberties (default: 20, 0 disables)
settings_v2.STARTING_MIN_LIBERTIES = BOARD_SIZE;    // Starting minimum liberties for each group.

    int player1_wins = 0;
    int player2_wins = 0;
    int draws = 0;
    const int NUM_GAMES = 40;
    const int THINK_TIME_MS = 6000;
    const int MAX_DEPTH_SEARCH = 5;

    for (int i = 0; i < NUM_GAMES; ++i) {
        Player winner;
        if (i % 2 == 0) {
            winner = playAIGame(settings_v1, "AI_v1", settings_v2, "AI_v2", true, THINK_TIME_MS, MAX_DEPTH_SEARCH);
            if (winner == BLACK) player1_wins++;
            else if (winner == WHITE) player2_wins++;
            else draws++;
        } else {
            winner = playAIGame(settings_v1, "AI_v1", settings_v2, "AI_v2", false, THINK_TIME_MS, MAX_DEPTH_SEARCH);
            if (winner == WHITE) player1_wins++;
            else if (winner == BLACK) player2_wins++;
            else draws++;
        }
    }

    std::cout << "\n--- Tournament Results ---" << std::endl;
    std::cout << "AI_v1 Wins: " << player1_wins << std::endl;
    std::cout << "AI_v2 Wins: " << player2_wins << std::endl;
    std::cout << "Draws: " << draws << std::endl;
    std::cout << "\nPress Enter to exit..." << std::endl; // Added
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    return 0;
}
