#include <iostream>
#include <sstream>
#include <chrono>
#include <random>
#include <string>

#include "Board.h"
#include "AtariGo.h"
#include "Globals.h"
#include "MiniMax.cpp"

Player playAIGame(const Settings& settingsPlayer1, const std::string& player1Name,
                  const Settings& settingsPlayer2, const std::string& player2Name,
                  bool player1PlaysBlack, int thinkingTimeMs, int maxDepth) {

    std::cout << "\n--- New Game ---" << std::endl;
    if (player1PlaysBlack) {
        std::cout << player1Name << " (Black) vs " << player2Name << " (White)" << std::endl;
    } else {
        std::cout << player2Name << " (Black) vs " << player1Name << " (White)" << std::endl;
    }

    Board board;

    settings = settingsPlayer1;
    AtariGo::calculateHeuristic(board);

    Player currentPlayerTurn = board.getPlayerToMove();

    int moveCount = 0;
    const int MAX_MOVES = BOARD_SIZE * 2;

    while (moveCount < MAX_MOVES) {
        moveCount++;
        std::cout << "\nMove: " << moveCount << ", Player to move: " << (currentPlayerTurn == BLACK ? "BLACK" : "WHITE") << std::endl;

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

        if (AtariGo::isTerminal(board)) {
            std::cout << "Game over. Terminal state reached." << std::endl;
            Player winner = NO_PLAYER;
            if (board.getHeuristic() == WIN) {
                 winner =  WHITE;
            } else if (board.getHeuristic() == -WIN) {
                 winner = BLACK;
            }

            if (winner != NO_PLAYER) {
                 std::cout << (winner == BLACK ? "BLACK" : "WHITE") << " wins!" << std::endl;
            }
            return winner;
        }

        MiniMax minimax_ai_agent;

        auto t0 = std::chrono::high_resolution_clock::now();
        Board bestMoveBoard = minimax_ai_agent.getBestMove(board, std::chrono::milliseconds(thinkingTimeMs), maxDepth);
        auto t1 = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        std::cout << "AI thought for " << ms << " ms." << std::endl;

        if (bestMoveBoard.getSignature() == board.getSignature()) {
            std::cout << "AI returned the same board state (Pass? Or error?). Game ends." << std::endl;
            Player otherPlayer = (currentPlayerTurn == BLACK) ? WHITE : BLACK;
            std::cout << (otherPlayer == BLACK ? "BLACK" : "WHITE") << " wins due to opponent pass/stall." << std::endl;
            return otherPlayer;
        }

        board = bestMoveBoard;
        currentPlayerTurn = board.getPlayerToMove();
    }
    std::cout << "Max moves reached. Game is a draw or ending by limit." << std::endl;
    return NO_PLAYER;
}


int main() {
    Settings settings_v1;
    settings_v1.TOTAL_LIB_MULTIPLIER = 0;
    settings_v1.STARTING_MIN_LIBERTIES = 8;
    //settings_v1.UNIQUE_LIB_MULTIPLIER = 1000;
    //settings_v1.STARTING_MIN_LIBERTIES = BOARD_SIZE;
    //settings_v1.UNIQUE_LIB_MULTIPLIER = 5;
    //settings_v1.SUCCESSOR_TRUNCATION_THRESHOLD = 1000;

    Settings settings_v2;
    settings_v2.TOTAL_LIB_MULTIPLIER = 0;
    //settings_v2.MIN_LIB_1_MULTIPLIER = 1000;
    //settings_v2.MIN_LIB_2_MULTIPLIER = settings_v2.MIN_LIB_1_MULTIPLIER / 4;
    //settings_v2.MIN_LIB_3_MULTIPLIER = settings_v2.MIN_LIB_2_MULTIPLIER / 4;
    //settings_v2.MIN_LIB_4_MULTIPLIER = settings_v2.MIN_LIB_3_MULTIPLIER / 4;
    //settings_v2.SUCCESSOR_TRUNCATION_THRESHOLD = 8;
    settings_v2.STARTING_MIN_LIBERTIES = 6;
    //settings_v2.UNIQUE_LIB_MULTIPLIER = 0;
    //settings_v1.SUCCESSOR_TRUNCATION_THRESHOLD = 6;

    int player1_wins = 0;
    int player2_wins = 0;
    int draws = 0;
    const int NUM_GAMES = 2;
    const int THINK_TIME_MS = 6000;
    const int MAX_DEPTH_SEARCH = 64;

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