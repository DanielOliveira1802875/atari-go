import { create } from "zustand";

const level = localStorage.getItem("level") ?? "4";
const boardEdge = localStorage.getItem("boardSize") ?? "9";
const wins =
  localStorage.getItem("reversiWins") ??
  JSON.stringify({
    level1: { ai: { black: 0, white: 0 }, player: { black: 0, white: 0 } },
    level2: { ai: { black: 0, white: 0 }, player: { black: 0, white: 0 } },
    level3: { ai: { black: 0, white: 0 }, player: { black: 0, white: 0 } },
    level4: { ai: { black: 0, white: 0 }, player: { black: 0, white: 0 } },
    level5: { ai: { black: 0, white: 0 }, player: { black: 0, white: 0 } },
    level6: { ai: { black: 0, white: 0 }, player: { black: 0, white: 0 } },
  });

type Store = {
  level: number;
  playerColor: TPlayer;
  boardEdge: number;
  wins: TWins;
  setLevel: (level: number) => void;
  setPlayer: (player: TPlayer) => void;
  setBoardSize: (size: number) => void;
  setWins: (wins: TWins) => void;
};

export interface LevelConfig {
  depth: number;
  time: number;
}
export const levelConfigs: Record<number, LevelConfig> = {
  1: { depth: 2, time: 1000 },
  2: { depth: 4, time: 1000 },
  3: { depth: 64, time: 1000 },
  4: { depth: 64, time: 3000 },
  5: { depth: 64, time: 6000 },
  6: { depth: 64, time: 12000 },
};

export const usePreferences = create<Store>()((set) => ({
  playerColor: "B",
  level: Number(level),
  boardEdge: Number(boardEdge),
  wins: JSON.parse(wins),
  setLevel(level) {
    set({ level });
    localStorage.setItem("level", String(level));
  },
  setPlayer(player) {
    set({ playerColor: player });
  },
  setBoardSize(boardSize) {
    set({ boardEdge: boardSize });
  },
  setWins(wins) {
    set({ wins });
    localStorage.setItem("wins", JSON.stringify(wins));
  },
}));

export type TCell = "." | "B" | "W";
export type TPlayer = "B" | "W";

export type TWins = {
  level1: {
    ai: {
      black: number;
      white: number;
    };
    player: {
      black: number;
      white: number;
    };
  };
  level2: {
    ai: {
      black: number;
      white: number;
    };
    player: {
      black: number;
      white: number;
    };
  };
  level3: {
    ai: {
      black: number;
      white: number;
    };
    player: {
      black: number;
      white: number;
    };
  };
  level4: {
    ai: {
      black: number;
      white: number;
    };
    player: {
      black: number;
      white: number;
    };
  };
  level5: {
    ai: {
      black: number;
      white: number;
    };
    player: {
      black: number;
      white: number;
    };
  };
  level6: {
    ai: {
      black: number;
      white: number;
    };
    player: {
      black: number;
      white: number;
    };
  };
};
