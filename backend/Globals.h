#ifndef GAMECOMMON_H
#define GAMECOMMON_H

enum Player { NO_PLAYER = 0, BLACK = 1, WHITE = 2 };
enum Stone { Empty = 0, Black = 1, White = 2 };
enum Bound { EXACT, LOWER, UPPER };

// Compile for different board sizes
#ifdef BOARD_EDGE_OVERRIDE
constexpr int BOARD_EDGE = BOARD_EDGE_OVERRIDE;
#else
constexpr int BOARD_EDGE = 9;
#endif

constexpr int BOARD_SIZE = BOARD_EDGE * BOARD_EDGE;
constexpr int WIN = 100'000'000;
constexpr int  ATARI_THREAT_SCORE = 1'000'000;
constexpr int  MIN_LIB_MULTIPLIER = 2'000;
constexpr int  UNIQUE_LIB_MULTIPLIER = 20;
constexpr int  STARTING_MIN_LIBERTIES = BOARD_SIZE;

// 128-bit unsigned integer type
using Bitboard128 = unsigned __int128;

// A 128-bit bitboard with only its least-significant bit set (…0001)
static constexpr Bitboard128 ONE_BIT = static_cast<Bitboard128>(1);

#endif