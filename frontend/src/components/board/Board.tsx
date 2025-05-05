import { useCallback, useEffect, useRef, useState } from "react";
import { Button } from "@/components/ui/button";
import BoardCell from "@/components/board/BoardCell.tsx";
import { levelConfigs, TCell, TPlayer, usePreferences } from "@/stores/usePreferences.ts";
import { getUINodePosition } from "@/lib/Helpers.ts";
import { Skeleton } from "@/components/ui/skeleton.tsx";
import { useNavigate } from "react-router";
import StatusMessage from "@/components/board/StatusMessage.tsx";
import AIThinkingProgress from "@/components/board/AIThinkingProgress.tsx";

const findLastMoveIndex = (oldB: TCell[], newB: TCell[]): number | null => {
  for (let i = 0; i < newB.length; i++) {
    if (oldB[i] === "." && (newB[i] === "B" || newB[i] === "W")) return i;
  }
  return null;
};

const getEmptyBoard = (size: number): TCell[] => {
  const b = Array(size).fill(".") as TCell[];
  /*     b[3 * boardEdge + 4] = "W";
     b[3 * boardEdge + 5] = "B";
     b[4 * boardEdge + 5] = "W";
     b[4 * boardEdge + 4] = "B";*/
  return b;
};

export default function Board() {
  const { playerColor, level, boardEdge } = usePreferences();

  const BOARD_SIZE = boardEdge * boardEdge;

  const [board, setBoard] = useState<TCell[]>(getEmptyBoard(BOARD_SIZE));
  const depthLimit = levelConfigs[level].depth;
  const timeLimitMs = levelConfigs[level].time;

  const [currentPlayer, setCurrentPlayer] = useState<TPlayer>("B");
  const [gameOver, setGameOver] = useState(false);
  const [statusMessage, setStatusMessage] = useState(<span>A carregar o motor de jogo...</span>);
  const [lastMoveIndex, setLastMoveIndex] = useState<number | null>(null);
  const [wasmLoading, setWasmLoading] = useState(true);
  const [aiThinking, setAiThinking] = useState(false);
  const capturedStonesIndexes = useRef<number[]>([]);
  const navigate = useNavigate();

  const workerRef = useRef<Worker | null>(null);

  const togglePlayer = useCallback(() => {
    setCurrentPlayer((prev) => (prev === "B" ? "W" : "B"));
  }, []);

  const checkWinner = useCallback((newBoard: TCell[]) => {
    console.log("checkWinner", newBoard);
    workerRef.current?.postMessage({ type: "checkCapture", payload: `${newBoard.join("")}` });
  }, []);

  useEffect(() => {
    const worker = new Worker(new URL("../../lib/wasm.worker.ts", import.meta.url), { type: "module" });
    worker.onmessage = ({ data }) => {
      const { type, payload } = data;
      if (type === "initialized") {
        setWasmLoading(false);
      }
      if (type === "moveDone") {
        const [boardStr] = payload.split(";");
        const newBoard = boardStr.split("") as TCell[];
        setBoard((prev) => {
          setLastMoveIndex(findLastMoveIndex(prev, newBoard));
          return newBoard;
        });
        checkWinner(newBoard);
      }
      if (type === "checkCaptureDone") {
        if (payload === "") {
          setAiThinking(false);
          togglePlayer();
          return;
        }
        console.log("checkCaptureDone", payload);
        const [boardStr, winnerStr] = payload.split(";");
        const indexes = boardStr.split(",") as TCell[];
        capturedStonesIndexes.current = indexes.map((index) => parseInt(index, 10));
        setGameOver(true);
        const winner = (winnerStr === "BLACK" ? "B" : "W") as TPlayer;
        setStatusMessage(<span>{`A partida terminou, as ${winner === "B" ? "pretas" : "brancas"} venceram!`}</span>);
      }
    };
    worker.postMessage({ type: "init" });
    workerRef.current = worker;
    return () => worker.terminate();
  }, [checkWinner, togglePlayer]);

  useEffect(() => {
    if (!wasmLoading && currentPlayer !== playerColor && !aiThinking && !gameOver) {
      setAiThinking(true);
      setStatusMessage(<span>Aguarde pela jogada da IA...</span>);
      workerRef.current?.postMessage({ type: "getBestMove", payload: `${board.join("")};${timeLimitMs};${depthLimit}` });
    }
    if (currentPlayer === playerColor && !aiThinking && !gameOver && !wasmLoading) {
      setStatusMessage(<span>Sua vez</span>);
    }
  }, [aiThinking, currentPlayer, wasmLoading, gameOver]);

  const handleCellClick = (idx: number) => {
    if (!playerColor || gameOver || wasmLoading || aiThinking || board[idx] !== "." || currentPlayer !== playerColor) return;
    setLastMoveIndex(idx);
    const newBoard = [...board];
    newBoard[idx] = playerColor;
    setBoard(newBoard);
    checkWinner(newBoard);
  };

  const isBoardInteractive = !wasmLoading && !gameOver && !aiThinking && !!playerColor;

  return (
    <div className="flex flex-col items-center gap-6">
      <div className="text-center">
        <StatusMessage>{statusMessage}</StatusMessage>
      </div>
      <div className="w-full max-w-md">
        <AIThinkingProgress timeLimitMs={timeLimitMs} isThinking={aiThinking && !gameOver} />
      </div>

      <div className="bg-gradient-to-br from-stone-300 to-stone-400 rounded-xl p-6 shadow-lg border relative border-stone-400 overflow-hidden">
        <div
          className={`transition-all duration-500 ease-in-out absolute top-0 bottom-0 left-0 right-0 z-20 ${
            isBoardInteractive ? "opacity-0 pointer-events-none" : "opacity-40  bg-zinc-900 pointer-events-auto"
          }`}
        />
        <div className={`grid grid-cols-9 w-[270px] h-[270px] relative sm:w-[450px] sm:h-[450px]`}>
          {board.map((cell, idx) => (
            <div key={idx} className="w-[30px] h-[30px] sm:w-[50px] sm:h-[50px] cursor-pointer" onClick={() => handleCellClick(idx)}>
              <BoardCell
                position={getUINodePosition(boardEdge, idx)}
                currentPlayer={currentPlayer}
                takenBy={cell}
                isLastMove={idx === lastMoveIndex}
                isCaptured={capturedStonesIndexes.current.includes(idx)}
              />
            </div>
          ))}
        </div>
      </div>
      <div>
        <Button size="lg" onClick={() => navigate("/")} className="bg-stone-800 hover:bg-stone-900">
          Voltar
        </Button>
        <Button
          size="lg"
          onClick={() => {
            capturedStonesIndexes.current = [];
            setLastMoveIndex(null);
            setBoard(getEmptyBoard(BOARD_SIZE));
            setGameOver(false);
            setCurrentPlayer("B");
            setAiThinking(false);
          }}
          className="bg-stone-800 hover:bg-stone-900 ml-4"
        >
          Reiniciar
        </Button>
      </div>
    </div>
  );
}
