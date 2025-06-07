#ifndef BBUTILS_H
#define BBUTILS_H

#include <array>
#include <cstdint>
#include <cassert>
#include <iostream>
#include "Globals.h"

/// Clears the least-significant set bit in b. UB if b==0.
inline void clearLSB(Bitboard128 &b) noexcept {
    assert(b != 0);
    // Subtracting 1 sets the least-significant 1-bit to 0 and sets all less-significant bits (to the right) to 1 (e.g., 1100 -> 1011)
    // AND-ing this with the original value then clears that 1-bit while leaving all other bits unchanged (e.g., 1100 & 1011 -> 1000)
    b &= b - 1;
}

/// Sets the bit at `pos` in `bb` (0..127) to 1.
inline void setBit(Bitboard128 &b, int pos) noexcept {
    assert(pos >= 0 && pos < 128);
    b |= (ONE_BIT << pos);
}

/// Clears the bit at `pos` in `bb` (0..127).
inline void clearBit(Bitboard128 &b, int pos) noexcept {
    assert(pos >= 0 && pos < 128);
    b &= ~(ONE_BIT << pos);
}

/// Tests whether the bit at `pos` in `bb` is set (1).
inline bool testBit(const Bitboard128 b, const int pos) noexcept {
    assert(pos >= 0 && pos < 128);
    return (b & (ONE_BIT << pos)) != 0;
}

/// Returns index [0..63] of the least-significant set bit in x.
inline int getLSBIndex(const uint64_t x) noexcept {
    assert(x != 0);
    // __builtin_ctzll is a GCC/Clang built-in function that counts the number of
    // trailing zeros in the binary representation of its argument.
    return __builtin_ctzll(x);
}

/// Returns index [0..127] of the least-significant set bit in b.
inline int getLSBIndex(const Bitboard128 b) noexcept {
    assert(b != 0);
    // Since __builtin_ctzll only works on 64-bit integers, we need to check
    // if the least significant bit is in the lower or upper 64 bits of b.
    const auto lo = static_cast<uint64_t>(b);
    if (lo != 0) {
        // LSB is in lower 64 bits
        return __builtin_ctzll(lo);
    }
    // LSB is in upper 64 bits; shift down and add 64
    const auto hi = static_cast<uint64_t>(b >> 64);
    return __builtin_ctzll(hi) + 64;
}

/// Clears and returns the index of the least-significant set bit in b.
inline int popLSB(Bitboard128 &b) noexcept {
    assert(b != 0);
    const int idx = getLSBIndex(b);
    clearLSB(b);
    return idx;
}

/// Returns the number of set bits in a 64-bit integer.
inline int bitCount(uint64_t x) noexcept {
    // __builtin_popcountll is a GCC/Clang built-in function that counts the number of set bits (population count) in its argument
    return __builtin_popcountll(x);
}

/// Returns the number of set bits in a 128-bit bitboard.
inline int bitCount(Bitboard128 b) noexcept {
    // Since __builtin_popcountll only works on 64-bit integers, we need to split the 128-bit integer into two 64-bit integers.
    const auto lo = static_cast<uint64_t>(b);
    const auto hi = static_cast<uint64_t>(b >> 64);
    return bitCount(lo) + bitCount(hi);
}

inline void printMask(Bitboard128 mask) {
    for (int row = 0; row < BOARD_EDGE; ++row) {
        for (int col = 0; col < BOARD_EDGE; ++col) {
            const char c = (mask & (ONE_BIT << (row * BOARD_EDGE + col))) ? '1' : '0';
            std::cout << c << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// Precomputed file masks for leftmost (A) and rightmost (I) columns

//  100 ... 0
//  100 ... 0
//  100 ... 0
//      ...
inline constexpr Bitboard128 FIRST_COLUMN_MASK = []() {
    Bitboard128 mask = 0;
    for (int row = 0; row < BOARD_EDGE; ++row)
        mask |= (ONE_BIT << (row * BOARD_EDGE + 0));
    return mask;
}();

//  0 ... 001
//  0 ... 001
//  0 ... 001
//    ...
inline constexpr Bitboard128 LAST_COLUMN_MASK = []() {
    Bitboard128 mask = 0;
    for (int row = 0; row < BOARD_EDGE; ++row)
        mask |= (ONE_BIT << (row * BOARD_EDGE + BOARD_EDGE - 1));
    return mask;
}();

inline constexpr Bitboard128 FULL_BOARD_MASK = ~static_cast<Bitboard128>(0) >> (128 - BOARD_SIZE);

inline Bitboard128 getNeighbourBits(const Bitboard128 bitboard) {
    // North: shift up BOARD_EDGE rows, then clamp off any bits > BOARD_SIZE-1
    const Bitboard128 north = (bitboard << BOARD_EDGE) & FULL_BOARD_MASK;

    // South: shift down BOARD_EDGE rows (shifting in zeros on top)
    const Bitboard128 south = bitboard >> BOARD_EDGE;

    // East/West: mask *before* shifting to prevent wrap-around
    const Bitboard128 east = (bitboard & ~LAST_COLUMN_MASK) << 1;
    const Bitboard128 west = (bitboard & ~FIRST_COLUMN_MASK) >> 1;

    // Combine
    return (north | south | east | west) & ~bitboard;
}

inline Bitboard128 getLibertyBits(const Bitboard128 bitboard, const Bitboard128 occupied) {
    return getNeighbourBits(bitboard) & ~occupied;
}

/// Returns a bitboard with all the bits in the same group as seed set to 1
inline Bitboard128 floodFillGroupBits(const Bitboard128 bitboard, const Bitboard128 seed) {
    Bitboard128 group = seed;
    while (true) {
        const Bitboard128 potential_additions = getNeighbourBits(group) & bitboard; // Neighbours of the current group that are also in the bitboard
        const Bitboard128 new_stones = potential_additions & ~group; // Only those not yet in the group

        if (!new_stones) break; // If no  new stones, break the loop
        group |= new_stones; // Add the new stones to the group
    }
    return group;
}

// array of mask for each move d4 d6 f4 f6 c3 c7 g3 g7 ( strong moves )
inline constexpr std::array<Bitboard128, 8> STRONG_MOVE_MASK = []() {
    if constexpr (BOARD_EDGE == 9) {
        std::array<Bitboard128, 8> m{};
        const int LE[] = {30, 48, 32, 50, 20, 24, 56, 60};
        for (int i = 0; i < 8; i++) {
            const int be = 80 - LE[i];
            m[i] = ONE_BIT << be;
        }
        return m;
    } else if constexpr (BOARD_EDGE == 7) {
        std::array<Bitboard128, 8> m{};
        const int LE[] = {16, 18, 30, 32};
        for (int i = 0; i < 4; i++) {
            const int be = 48 - LE[i];
            m[i] = ONE_BIT << be;
        }
        return m;
    } else {
        static_assert(BOARD_EDGE == 9 || BOARD_EDGE == 7, "Unsupported BOARD_EDGE size for STRONG_MOVE_MASK");
        return std::array<Bitboard128, 8>{};
    }
}();


#endif
