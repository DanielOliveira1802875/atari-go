#include <iostream>
#include <sstream>
#include <chrono>
#include <random>

#include "Board.h"
#include "AtariGo.h"
#include "Globals.h"
#include "MiniMax.cpp"

class Game {
public:
    static void run() {
        std::mt19937_64 rng;
        Board board;
        AtariGo atariGo;

        board.setStone(3, 5);
        board.setStone(3, 4);
        board.setStone(4, 4);
        board.setStone(4, 5);
        AtariGo::computeHeuristic(board);

        constexpr Player human = BLACK;
        constexpr Player computer = WHITE;
        Player turn = board.getPlayerToMove();

        AtariGo::print(board);

        while (true) {
            AtariGo::computeHeuristic(board);
            AtariGo::print(board);

            if (AtariGo::isTerminal(board)) {
                std::cout << "Game over. Terminal state reached.\n";
                std::cout << (board.getHeuristic() > 0 ? "WHITE" : "BLACK") << " wins!\n";
                break;
            }

            if (turn == human) {
                humanMove(board);
                turn = computer;
            }
            else {
                MiniMax minimax;
                std::cout << "Player " << (turn == BLACK ? "BLACK" : "WHITE") << " (AI) is thinking...\n";
                auto t0 = std::chrono::high_resolution_clock::now();

                Board best = minimax.getBestMove(board, std::chrono::milliseconds(5000), 64);

                auto t1 = std::chrono::high_resolution_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
                std::cout << "AI thought for " << ms << " ms\n";

                if (best.getSignature() == board.getSignature()) {
                     std::cout << "AI returned the same board state. Passing? Or error?\n";
                }

                board = best;
                turn = (turn == BLACK ? WHITE : BLACK);
            }
        }
    }

private:
    static void humanMove(Board& board) {
        int row = -1, col = -1;
        bool validMove = false;

        while (!validMove) {
            std::cout << "Enter your move (row col) or (letter number): ";
            std::string input;
            std::getline(std::cin, input);

            if (input.length() >= 2 && std::isalpha(input[0])) {
                char rowChar = std::toupper(input[0]);
                row = rowChar - 'A';

                std::string colStr = input.substr(1);
                std::istringstream iss(colStr);
                if (!(iss >> col)) {
                    std::cout << "Invalid column format. Please enter a row letter followed by column number (e.g., C6).\n";
                    continue;
                }
                col--;
            } else {
                std::istringstream iss(input);
                if (!(iss >> row >> col)) {
                    std::cout << "Invalid format. Try either 'C 6' or 'C6' or '2 5'.\n";
                    continue;
                }
            }

            if (!Board::isInBounds(row, col)) {
                std::cout << "Move out of bounds. The board is " << BOARD_EDGE << "x" << BOARD_EDGE << ".\n";
                continue;
            }

            if (!board.isEmpty(row, col)) {
                std::cout << "Position already occupied. Choose an empty cell.\n";
                continue;
            }

            validMove = true;
        }

        board.setStone(row, col);
        AtariGo::computeHeuristic(board);
        std::cout << "You placed a stone at (" << row << ", " << col << ")\n";
    }
};

int main() {
    Game::run();
    return 0;
}

