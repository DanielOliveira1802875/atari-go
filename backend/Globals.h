#ifndef GAMECOMMON_H
#define GAMECOMMON_H

enum Player { BLACK = 1, WHITE = 2 };
enum Stone { Empty = 0, Black = 1, White = 2 };
enum Bound { EXACT, LOWER, UPPER };

constexpr int BOARD_EDGE = 9;
constexpr int BOARD_SIZE = BOARD_EDGE * BOARD_EDGE;

constexpr int WIN = 1'000'000;
constexpr int ATARI_PENALTY = 200'000;
constexpr int NEAR_ATARI_PENALTY = 100'000;

// 128-bit unsigned integer type
using Bitboard128 = unsigned __int128;

// A 128-bit bitboard with only its least-significant bit set (…0001)
static constexpr Bitboard128 ONE_BIT = static_cast<Bitboard128>(1);

#endif