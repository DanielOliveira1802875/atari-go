#ifndef ZOBRIST_H
#define ZOBRIST_H
#include <cstdint>
#include "Globals.h"
#include <random>


inline uint64_t ZOBRIST_TABLE[2][BOARD_SIZE];
static bool zobrist_initialised = false;

inline void initZobrist()
{
    if (zobrist_initialised) return;
    std::mt19937_64 rng(123456789ULL);
    std::uniform_int_distribution<uint64_t> d;

    for (auto & colourTable : ZOBRIST_TABLE)
        for (uint64_t & cell : colourTable)
            cell = d(rng);

    zobrist_initialised = true;
}

#endif
