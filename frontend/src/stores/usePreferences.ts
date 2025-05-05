import { create } from "zustand";

const level = localStorage.getItem("level") ?? "4";
const boardEdge = localStorage.getItem("boardSize") ?? "9";
const defaultPlayer = localStorage.getItem("player") ?? "W";

type Store = {
  level: number;
  playerColor: TPlayer;
  boardEdge: number;
  setLevel: (level: number) => void;
  setPlayer: (player: TPlayer) => void;
  setBoardSize: (size: number) => void;
};

export interface LevelConfig {
  depth: number;
  time: number;
}
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
  setLevel(level) {
    set({ level });
    localStorage.setItem("level", String(level));
  },
  setPlayer(player) {
    set({ playerColor: player });
    localStorage.setItem("player", player);
  },
  setBoardSize(boardSize) {
    set({ boardEdge: boardSize });
  },
}));

export type TCell = "." | "B" | "W";
export type TPlayer = "B" | "W";
