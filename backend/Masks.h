/**
* @file NeighborMasks.h
 * @brief Precompute orthogonal neighbor bitmasks for a BOARD_EDGE×BOARD_EDGE grid.
 *
 * This module builds a lookup table ORTH_MASK of size BOARD_SIZE, where each entry
 * ORTH_MASK[pos] is a Bitboard128 with bits set for the up/down/left/right neighbors
 * of square `pos`.  The initNeighborMasks() function must be called once at startup
 * to fill the table.
 *
 * Usage example:
 *
 *   initNeighborMasks(); // Initialize the masks once
 *
 *   On a 9×9 board (BOARD_EDGE = 9), pos = 40 is at (row=4, col=4), the following bits are set:
 *      up    = pos - 9  = 31
 *      down  = pos + 9  = 49
 *      left  = pos - 1  = 39
 *      right = pos + 1  = 41
 *
 *   Bitboard128 mask = ORTH_MASK[40]; // mask now has exactly those four bits set, all others zero.
 *
 *   Bitboard128 occupancy = black_board | white_board; // Gives the occupancy of the board
 *
 *   To test whether any orthogonal neighbor of `pos` is occupied:
 *   if (occupancy & mask) {
 *       // at least one neighbor is occupied (result of & is non-zero)
 *   } else {
 *       // all four neighbors are empty (result of & is zero)
 *   }
 */

#ifndef NEIGHBOR_MASKS_H
#define NEIGHBOR_MASKS_H

#include <iostream>

#include "Globals.h"

inline void printMask(Bitboard128 mask) {
    for (int row = 0; row < BOARD_EDGE; ++row) {
        for (int col = 0; col < BOARD_EDGE; ++col) {
            char c = (mask & (ONE_BIT << (row * BOARD_EDGE + col))) ? '1' : '0';
            std::cout << c << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// Precomputed orthogonal neighbor masks for each square on the board
inline Bitboard128 NEIGHBOR_MASK[BOARD_SIZE];
inline bool neighborMasksInitialized = false;

inline void initNeighborMasks() {
    if (neighborMasksInitialized) return; // Avoid re-initialization
    for (int pos = 0; pos < BOARD_SIZE; ++pos) {
        int row = pos / BOARD_EDGE; // 0 = top row, BOARD_EDGE−1 = bottom row
        int col = pos % BOARD_EDGE; // 0 = leftmost column, BOARD_EDGE−1 = rightmost

        Bitboard128 m = 0; // start with empty mask

        // If there is a row above, set the bit for the “up” neighbor
        if (row > 0) {
            m |= (ONE_BIT << (pos - BOARD_EDGE)); // shift ONE_BIT down by BOARD_EDGE to mark square above
        }

        // If there is a row below, set the bit for the “down” neighbor
        if (row < BOARD_EDGE - 1) {
            m |= (ONE_BIT << (pos + BOARD_EDGE)); // shift ONE_BIT up by BOARD_EDGE to mark square below
        }

        // If there is a column to the left, set the bit for the “left” neighbor
        if (col > 0) {
            m |= (ONE_BIT << (pos - 1)); // shift ONE_BIT right by one to mark square to the left
        }

        // If there is a column to the right, set the bit for the “right” neighbor
        if (col < BOARD_EDGE - 1) {
            m |= (ONE_BIT << (pos + 1)); // shift ONE_BIT left by one to mark square to the right
        }

        // Store the combined mask of all orthogonal neighbors for this square
        NEIGHBOR_MASK[pos] = m;
        neighborMasksInitialized = true;
    }
}

// Precomputed file masks for leftmost (A) and rightmost (I) columns

//  100 ... 0
//  100 ... 0
//  100 ... 0
//      ...
inline Bitboard128 FIRST_COLUMN_MASK = []() {
    Bitboard128 mask = 0;
    for (int row = 0; row < BOARD_EDGE; ++row)
        mask |= (ONE_BIT << (row * BOARD_EDGE + 0));
    return mask;
}();

//  0 ... 001
//  0 ... 001
//  0 ... 001
//    ...
inline Bitboard128 LAST_COLUMN_MASK = []() {
    Bitboard128 mask = 0;
    for (int row = 0; row < BOARD_EDGE; ++row)
        mask |= (ONE_BIT << (row * BOARD_EDGE + BOARD_EDGE - 1));
    return mask;
}();

inline Bitboard128 FULL_BOARD_MASK =  ~static_cast<Bitboard128>(0) >> (128 - BOARD_SIZE);


#endif
