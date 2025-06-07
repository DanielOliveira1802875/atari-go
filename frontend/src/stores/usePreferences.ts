import { create } from "zustand";
import { parseStatistics, getEmptyStatistics, TBoardWinLoss } from "@/stores/statistics.ts";

// get the level from localStorage or default to 4
let level = localStorage.getItem("level") ?? "4";
if (level !== "1" && level !== "2" && level !== "3" && level !== "4" && level !== "5" && level !== "6") level = "4"; // Default to 4 if invalid value

// get the board size from localStorage or default to 9
let boardEdge = localStorage.getItem("boardSize") ?? "9";
if (boardEdge !== "7" && boardEdge !== "8" && boardEdge !== "9") boardEdge = "9"; // Default to 9 if invalid value

// get the player from localStorage or default to "W"
let defaultPlayer = localStorage.getItem("player") ?? "W";
if (defaultPlayer !== "W" && defaultPlayer !== "B") defaultPlayer = "W"; // Default to W if invalid value

// Parse the statistics from localStorage or default to empty statistics
const statistics = parseStatistics(localStorage.getItem("statistics") ?? "{}");

type Store = {
  level: number;
  playerColor: TPlayer;
  boardEdge: number;
  statistics: TBoardWinLoss;
  setLevel: (level: number) => void;
  setPlayer: (player: TPlayer) => void;
  setBoardEdge: (size: number) => void;
  addWin: () => void;
  addLoss: () => void;
  clearStatistics: () => void;
};

export const levelConfigs: Record<number, LevelConfig> = {
  1: { depth: 1, time: 200 },
  2: { depth: 3, time: 500 },
  3: { depth: 5, time: 800 },
  4: { depth: 64, time: 1000 },
  5: { depth: 64, time: 3000 },
  6: { depth: 64, time: 6000 },
};

export const usePreferences = create<Store>()((set) => ({
  playerColor: defaultPlayer as TPlayer,
  level: Number(level),
  boardEdge: Number(boardEdge),
  statistics,
  setLevel(level) {
    set({ level });
    localStorage.setItem("level", String(level));
  },
  setPlayer(player) {
    set({ playerColor: player });
    localStorage.setItem("player", player);
  },
  setBoardEdge(boardEdge) {
    set({ boardEdge: boardEdge });
    localStorage.setItem("boardSize", String(boardEdge));
  },
  addWin() {
    set((state) => {
      const newStats = { ...state.statistics };
      newStats[state.boardEdge][state.playerColor][state.level].wins += 1;
      localStorage.setItem("statistics", JSON.stringify(newStats));
      return { statistics: newStats };
    });
  },
  addLoss() {
    set((state) => {
      const newStats = { ...state.statistics };
      newStats[state.boardEdge][state.playerColor][state.level].losses += 1;
      localStorage.setItem("statistics", JSON.stringify(newStats));
      return { statistics: newStats };
    });
  },
  clearStatistics: () => {
    set({ statistics: getEmptyStatistics() });
    localStorage.setItem("statistics", JSON.stringify(getEmptyStatistics()));
  },
}));

export interface LevelConfig {
  depth: number;
  time: number;
}

export type TCell = "." | "B" | "W";
export type TPlayer = "B" | "W";
