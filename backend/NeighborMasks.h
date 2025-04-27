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

#include "Globals.h"

// Precomputed orthogonal neighbor masks for each square on the board
inline Bitboard128 ORTH_MASK[BOARD_SIZE];

inline void initNeighborMasks() {
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
        ORTH_MASK[pos] = m;
    }
}

#endif
