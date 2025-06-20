#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <stdexcept>

#include "BBUtils.h"
#include "Globals.h"

class Board {
private:
    Bitboard128 black_board;
    Bitboard128 white_board;
    uint64_t zobrist_hash;
    unsigned char turn;
    bool isHeuristicCalculated;
    int heuristic;

    // Convert (row, col) into a single bit‐index: row-major, 0 ≤ row,col < BOARD_EDGE
    static constexpr int pos_from_coord(const int row, const int col) {
        // constexpr
        return row * BOARD_EDGE + col;
    }

public:
    Board() : black_board(0), white_board(0), zobrist_hash(0), turn(1), isHeuristicCalculated(false), heuristic(0) {
    }

    void setHeuristic(const int h) {
        heuristic = h;
        isHeuristicCalculated = true;
    }

    void invalidateHeuristic() {
        isHeuristicCalculated = false;
    }

    void setStone(const int row, const int col) { setStone(pos_from_coord(row, col)); }

    [[nodiscard]] bool isEmpty(const int row, const int col) const { return isEmpty(pos_from_coord(row, col)); }

    [[nodiscard]] static constexpr bool isInBounds(const int row, const int col) {
        return row >= 0 && row < BOARD_EDGE && col >= 0 && col < BOARD_EDGE;
    }

    [[nodiscard]] Stone getStone(const int row, const int col) const { return getStone(pos_from_coord(row, col)); }

    [[nodiscard]] bool isBlack(const int pos) const { return testBit(black_board, pos); }
    [[nodiscard]] bool isWhite(const int pos) const { return testBit(white_board, pos); }

    [[nodiscard]] bool isEmpty(const int pos) const { return !testBit(black_board | white_board, pos); }

    [[nodiscard]] Stone getStone(const int pos) const {
        if (isBlack(pos))
            return Black;
        if (isWhite(pos))
            return White;
        return Empty;
    }

    [[nodiscard]] Player getPlayerToMove() const { return (turn & 1) ? BLACK : WHITE; }
    [[nodiscard]] unsigned char getTurn() const { return turn; }

    void setStone(const int pos) {
        if (getPlayerToMove() == BLACK)
            setBlack(pos);
        else
            setWhite(pos);
    }

    void setBlack(const int pos) {
        if (!isEmpty(pos))
            throw std::runtime_error("Position already occupied.");
        setBit(black_board, pos);
        // XOR the Zobrist hash with the precomputed random value for Black on square `pos`
        zobrist_hash ^= ZOBRIST_TABLE[0][pos];
        ++turn;
        isHeuristicCalculated = false;
    }

    void setWhite(const int pos) {
        if (!isEmpty(pos))
            throw std::runtime_error("Position already occupied.");
        setBit(white_board, pos);
        // XOR the Zobrist hash with the precomputed random value for White on square `pos`
        zobrist_hash ^= ZOBRIST_TABLE[1][pos];
        ++turn;
        isHeuristicCalculated = false;
    }

    [[nodiscard]] bool getIsHeuristicCalculated() const { return isHeuristicCalculated; }

    [[nodiscard]] int getHeuristic() const {
        // assert(isHeuristicCalculated);
        return heuristic;
    }

    [[nodiscard]] uint64_t getSignature() const { return zobrist_hash; }

    [[nodiscard]] Bitboard128 getBlackBits() const { return black_board; }
    [[nodiscard]] Bitboard128 getWhiteBits() const { return white_board; }
    [[nodiscard]] Bitboard128 getOccupiedBits() const { return black_board | white_board; }
};

#endif
