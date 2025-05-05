#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <array>
#include <cstdint>
#include "Globals.h"

// A constexpr xorshift64 PRNG (seeded with a fixed constant)
constexpr uint64_t xorshift64(uint64_t& state) {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

// Build a 2×BOARD_SIZE table at compile time
constexpr std::array<std::array<uint64_t, BOARD_SIZE>, 2> generateZobristTable() {
    std::array<std::array<uint64_t, BOARD_SIZE>, 2> table{};
    uint64_t state = 123456789ULL;  // fixed seed

    for (size_t color = 0; color < 2; ++color) {
        for (size_t pos = 0; pos < BOARD_SIZE; ++pos) {
            table[color][pos] = xorshift64(state);
        }
    }
    return table;
}

// Now fully initialized at compile time; no init function needed
inline constexpr auto ZOBRIST_TABLE = generateZobristTable();

#endif // ZOBRIST_H
