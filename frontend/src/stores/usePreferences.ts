import { create } from "zustand";

const level = localStorage.getItem("level") ?? "4";
const boardEdge = localStorage.getItem("boardSize") ?? "9";
const defaultPlayer = localStorage.getItem("player") ?? "W";

const statistics = localStorage.getItem("statistics")
  ? JSON.parse(localStorage.getItem("statistics")!)
  : ({
      W: {
        1: { wins: 0, losses: 0 },
        2: { wins: 0, losses: 0 },
        3: { wins: 0, losses: 0 },
        4: { wins: 0, losses: 0 },
        5: { wins: 0, losses: 0 },
        6: { wins: 0, losses: 0 },
      },
      B: {
        1: { wins: 0, losses: 0 },
        2: { wins: 0, losses: 0 },
        3: { wins: 0, losses: 0 },
        4: { wins: 0, losses: 0 },
        5: { wins: 0, losses: 0 },
        6: { wins: 0, losses: 0 },
      },
    } as TStatistics);

type Store = {
  level: number;
  playerColor: TPlayer;
  boardEdge: number;
  statistics: TStatistics;
  setLevel: (level: number) => void;
  setPlayer: (player: TPlayer) => void;
  setBoardEdge: (size: number) => void;
  addWin: () => void;
  addLoss: () => void;
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
  },
  addWin() {
    set((state) => {
      const newStats = { ...state.statistics };
      newStats[state.playerColor][state.level].wins += 1;
      localStorage.setItem("statistics", JSON.stringify(newStats));
      return { statistics: newStats };
    });
  },
  addLoss() {
    set((state) => {
      const newStats = { ...state.statistics };
      newStats[state.playerColor][state.level].losses += 1;
      localStorage.setItem("statistics", JSON.stringify(newStats));
      return { statistics: newStats };
    });
  },
}));

export interface LevelConfig {
  depth: number;
  time: number;
}
export type TStatistics = Record<TPlayer, Record<number, { wins: number; losses: number }>>;
export type TCell = "." | "B" | "W";
export type TPlayer = "B" | "W";
