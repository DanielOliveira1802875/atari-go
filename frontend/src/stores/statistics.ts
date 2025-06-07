import { TPlayer } from "@/stores/usePreferences.ts";
import { MAX_LEVEL } from "@/components/board/LevelSlider.tsx";

type TWinLoss = { wins: number; losses: number };
type TLevelWinLoss = Record<number, TWinLoss>;
type TPlayerWinLoss = Record<TPlayer, TLevelWinLoss>;
export type TBoardWinLoss = Record<number, TPlayerWinLoss>;

// Creating an empty result for wins and losses.
const emptyResult: TWinLoss = { wins: 0, losses: 0 };
const getEmptyLevels = () =>
  Array.from({ length: MAX_LEVEL }, () => emptyResult).reduce((acc, _, index) => {
    acc[index + 1] = { ...emptyResult };
    return acc;
  }, {} as TLevelWinLoss);

// Creates an empty statistics object for all board sizes (7, 8, and 9).
export const getEmptyStatistics = () => ({
  7: { W: { ...getEmptyLevels() }, B: { ...getEmptyLevels() } },
  8: { W: { ...getEmptyLevels() }, B: { ...getEmptyLevels() } },
  9: { W: { ...getEmptyLevels() }, B: { ...getEmptyLevels() } },
});

// Validates the statistics format to ensure it has the correct structure and levels.
const validateStatistics = (data: TBoardWinLoss): boolean => {
  try {
    if (Object.keys(data[7].W).length !== MAX_LEVEL || Object.keys(data[7].B).length !== MAX_LEVEL) return false;
    if (Object.keys(data[8].W).length !== MAX_LEVEL || Object.keys(data[8].B).length !== MAX_LEVEL) return false;
    if (Object.keys(data[9].W).length !== MAX_LEVEL || Object.keys(data[9].B).length !== MAX_LEVEL) return false;
  } catch (e) {
    if (import.meta.env.DEV) console.error("Invalid statistics format:", e);
    return false;
  }
  return true;
};

// Parses the statistics from a string and validates the format, if invalid or parsing fails, returns empty statistics.
export const parseStatistics = (data: string): TBoardWinLoss => {
  try {
    const parsed = JSON.parse(data);
    if (validateStatistics(parsed)) {
      return parsed;
    } else {
      if (import.meta.env.DEV) console.warn("Invalid statistics format, returning empty statistics.");
    }
  } catch (e) {
    if (import.meta.env.DEV) console.error("Failed to parse statistics:", e);
  }
  return getEmptyStatistics();
};
