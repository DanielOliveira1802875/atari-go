#ifndef BBUTILS_H
#define BBUTILS_H

#include <cstdint>
#include <cassert>
#include "Globals.h"

class  BBUtils {
public:

    /// Sets the bit at `pos` in `bb` (0..127) to 1.
    static inline void setBit(Bitboard128 &b, int pos) noexcept {
        assert(pos >= 0 && pos < 128);
        b |= (ONE_BIT << pos);
    }

    /// Tests whether the bit at `pos` in `bb` is set (1).
    static inline bool testBit(Bitboard128 b, int pos) noexcept {
        assert(pos >= 0 && pos < 128);
        return (b & (ONE_BIT << pos)) != 0;
    }

    /// Returns index [0..63] of the least-significant set bit in x.
    static inline int getLSBIndex(uint64_t x) noexcept {
        assert(x != 0);
        // __builtin_ctzll is a GCC/Clang built-in function that counts the number of
        // trailing zeros in the binary representation of its argument.
        return __builtin_ctzll(x);
    }

    /// Returns index [0..127] of the least-significant set bit in b.
    static inline int getLSBIndex(Bitboard128 b) noexcept {
        assert(b != 0);
        // Since __builtin_ctzll only works on 64-bit integers, we need to check
        // if the least significant bit is in the lower or upper 64 bits of b.
        auto lo = static_cast<uint64_t>(b);
        if (lo) {
            // LSB is in lower 64 bits
            return __builtin_ctzll(lo);
        }
        // LSB is in upper 64 bits; shift down and add 64
        auto hi = static_cast<uint64_t>(b >> 64);
        return __builtin_ctzll(hi) + 64;
    }

    /// Clears and returns the index of the least-significant set bit in b.
    static inline int popLSB(Bitboard128 &b) noexcept {
        assert(b != 0);
        int idx = getLSBIndex(b);
        clearLSB(b);
        return idx;
    }

    /// Clears the least-significant set bit in b. UB if b==0.
    static inline void clearLSB(Bitboard128 &b) noexcept {
        assert(b != 0);
        // Subtracting 1 sets the least-significant 1-bit to 0 and sets all less-significant bits (to the right) to 1 (e.g., 1100 -> 1011)
        // AND-ing this with the original value then clears that 1-bit while leaving all other bits unchanged (e.g., 1100 & 1011 -> 1000)
        b &= b - 1;
    }

    /// Returns the number of set bits in a 64-bit integer.
    static inline int bitCount(uint64_t x) noexcept {
        // __builtin_popcountll is a GCC/Clang built-in function that counts the number of set bits (population count) in its argument
        return __builtin_popcountll(x);
    }

    /// Returns the number of set bits in a 128-bit bitboard.
    static inline int bitCount(Bitboard128 b) noexcept {
        // Since __builtin_popcountll only works on 64-bit integers, we need to split the 128-bit integer into two 64-bit integers.
        const auto lo = static_cast<uint64_t>(b);
        const auto hi = static_cast<uint64_t>(b >> 64);
        return bitCount(lo) + bitCount(hi);
    }

};

#endif
