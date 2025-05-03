import { useEffect, useRef, useState } from "react";
import { Slider } from "@/components/ui/slider";
import { Button } from "@/components/ui/button";
import BoardCell from "@/components/board/BoardCell.tsx";
import { Table, TableHeader, TableRow, TableHead, TableBody, TableCell } from "@/components/ui/table";
import { levelConfigs, TCell, TPlayer, usePreferences } from "@/stores/usePreferences.ts";
import { getUINodePosition } from "@/lib/Helpers.ts";
import LevelSlider from "@/components/board/LevelSlider.tsx";
import { Skeleton } from "@/components/ui/skeleton.tsx";

const findLastMoveIndex = (oldB: TCell[], newB: TCell[]): number | null => {
  for (let i = 0; i < newB.length; i++) {
    if (oldB[i] === "." && (newB[i] === "B" || newB[i] === "W")) return i;
  }
  return null;
};

export default function Board() {
  const { playerColor, level, boardEdge, ...gamePrefs } = usePreferences();

  const BOARD_SIZE = boardEdge * boardEdge;

  const [board, setBoard] = useState<TCell[]>(() => {
    const b = Array(BOARD_SIZE).fill(".") as TCell[];
    /* b[3 * boardEdge + 4] = "W";
        b[3 * boardEdge + 5] = "B";
        b[4 * boardEdge + 5] = "W";
        b[4 * boardEdge + 4] = "B";*/
    return b;
  });

  const depthLimit = levelConfigs[level].depth;
  const timeLimitMs = levelConfigs[level].time;

  const [currentPlayer, setCurrentPlayer] = useState<TPlayer>("B");
  const [gameOver, setGameOver] = useState(false);
  const [statusMessage, setStatusMessage] = useState(<span>Loading game engine...</span>);
  const [lastMoveIndex, setLastMoveIndex] = useState<number | null>(null);
  const [wasmLoading, setWasmLoading] = useState(true);
  const [aiThinking, setAiThinking] = useState(false);

  const workerRef = useRef<Worker | null>(null);

  useEffect(() => {
    const worker = new Worker(new URL("../../lib/wasm.worker.ts", import.meta.url), { type: "module" });
    worker.onmessage = ({ data }) => {
      const { type, payload } = data;
      if (type === "initialized") {
        setWasmLoading(false);
        setStatusMessage(<span>Choose your color to start</span>);
      }
      if (type === "moveDone") {
        const [boardStr, winnerStr] = payload.split(";");
        const newBoard = boardStr.split("") as TCell[];
        setBoard((prev) => {
          setLastMoveIndex(findLastMoveIndex(prev, newBoard));
          return newBoard;
        });
        setAiThinking(false);
        setCurrentPlayer((prev) => (prev === "B" ? "W" : "B"));
        if (winnerStr) {
          setGameOver(true);
          const winner = (winnerStr === "BLACK" ? "B" : "W") as TPlayer;
          setStatusMessage(<span>{`Game Over! ${winner === "B" ? "Black" : "White"} wins!`}</span>);
          const isPlayerWin = winner === playerColor;
          /*setWins((w) => ({
            playerBlack: w.playerBlack + (isPlayerWin && winner === "B" ? 1 : 0),
            playerWhite: w.playerWhite + (isPlayerWin && winner === "W" ? 1 : 0),
            aiBlack: w.aiBlack + (!isPlayerWin && winner === "B" ? 1 : 0),
            aiWhite: w.aiWhite + (!isPlayerWin && winner === "W" ? 1 : 0),
          }));*/
        }
      }
    };
    worker.postMessage({ type: "init" });
    workerRef.current = worker;
    return () => worker.terminate();
  }, []);

  useEffect(() => {
    if (!wasmLoading && playerColor && currentPlayer !== playerColor && !gameOver) {
      setAiThinking(true);
      setStatusMessage(<span>AI is thinking...</span>);
      workerRef.current?.postMessage({ type: "getBestMove", payload: `${board.join("")};${timeLimitMs};${depthLimit}` });
    }
    if (playerColor && currentPlayer === playerColor && !aiThinking && !gameOver && !wasmLoading) {
      setStatusMessage(
        <span>
          Your turn <BoardCell position={"middle"} hideGrid currentPlayer={playerColor} takenBy={playerColor} />
        </span>,
      );
    }
  }, [currentPlayer, playerColor, wasmLoading]);

  const handleCellClick = (idx: number) => {
    if (!playerColor || gameOver || wasmLoading || aiThinking || board[idx] !== "." || currentPlayer !== playerColor) return;
    setLastMoveIndex(idx);
    const newBoard = [...board];
    newBoard[idx] = playerColor;
    setBoard(newBoard);
    setCurrentPlayer(playerColor === "B" ? "W" : "B");
  };

  const isBoardInteractive = !wasmLoading && !gameOver && !aiThinking && !!playerColor;

  return (
    <div className="flex flex-col items-center gap-6">
      {!playerColor && (
        <div className="fixed inset-0 bg-black/60 z-20 flex flex-col items-center justify-center gap-4 text-white">
          <h2 className="text-2xl font-bold">Choose your color</h2>
          <div className="flex gap-6">
            <Button
              size="lg"
              onClick={() => {
                gamePrefs.setPlayer("B");
                setCurrentPlayer("B");
              }}
            >
              Play as Black
            </Button>
            <Button
              size="lg"
              onClick={() => {
                gamePrefs.setPlayer("W");
                setCurrentPlayer("B");
              }}
            >
              Play as White
            </Button>
          </div>
        </div>
      )}

      <LevelSlider disabled={aiThinking} />

      <div className="text-center">
        <div className="font-bold text-lg min-h-[1.5rem]">{statusMessage}</div>
      </div>

      <div className="bg-gradient-to-br from-stone-300 to-stone-400 rounded-xl p-6 shadow border relative">
        {!isBoardInteractive && <Skeleton className="absolute cursor-not-allowed z-50 opacity-60 bg-stone-400 top-0 bottom-0 left-0 right-0" />}
        <div className={`grid grid-cols-9 w-[450px] h-[450px] relative`}>
          {board.map((cell, idx) => (
            <div key={idx} className="w-[50px] h-[50px] cursor-pointer" onClick={() => handleCellClick(idx)}>
              <BoardCell position={getUINodePosition(boardEdge, idx)} currentPlayer={currentPlayer} takenBy={cell} isLastMove={idx === lastMoveIndex} />
            </div>
          ))}
        </div>
      </div>

      {/* Statistics Table */}
      <Table>
        {/*<TableCaption>Game Win Statistics</TableCaption>*/}
        <TableHeader>
          <TableRow>
            <TableHead></TableHead>
            <TableHead>
              <div className="size-4">
                <BoardCell position={"middle"} hideGrid currentPlayer={"B"} takenBy={"B"} />
              </div>
            </TableHead>
            <TableHead>
              <div className="size-4">
                <BoardCell position={"middle"} hideGrid currentPlayer={"W"} takenBy={"W"} />
              </div>
            </TableHead>
          </TableRow>
        </TableHeader>
        <TableBody>
          <TableRow>
            <TableCell className="font-medium">Jogador</TableCell>
            <TableCell>{0}</TableCell>
            <TableCell>{0}</TableCell>
          </TableRow>
          <TableRow>
            <TableCell className="font-medium">IA</TableCell>
            <TableCell>{0}</TableCell>
            <TableCell>{0}</TableCell>
          </TableRow>
        </TableBody>
      </Table>
    </div>
  );
}
