import { useEffect, useRef, useState } from "react";
import UINode from "@/components/board/UINode";
import { getUINodePosition } from "@/components/board/Helpers";

export type TCell = "." | "B" | "W";
export type TPlayer = "B" | "W";

export default function Board() {
  const size = 9;
  const BOARD_SIZE = size * size;

  // Board state
  const [board, setBoard] = useState<TCell[]>(Array(BOARD_SIZE).fill("."));
  const [currentPlayer, setCurrentPlayer] = useState<TPlayer>("B");
  const [gameOver, setGameOver] = useState(false);
  const [winner, setWinner] = useState<TPlayer | null>(null);
  const [wasmLoading, setWasmLoading] = useState(true);
  const [aiThinking, setAiThinking] = useState(false);
  const [statusMessage, setStatusMessage] = useState("Loading game engine...");

  // Reference for Web Worker
  const workerRef = useRef<Worker | null>(null);

  const togglePlayer = () => {
    setCurrentPlayer((prev) => (prev === "B" ? "W" : "B"));
  };

  // Initialize the Web Worker
  useEffect(() => {
    const worker = new Worker(new URL("../../lib/wasm.worker.ts", import.meta.url), { type: "module" });

    worker.onmessage = (event) => {
      const { type, payload } = event.data;

      if (type === "initialized") {
        setWasmLoading(false);
        setStatusMessage(`Your turn (Black)`);
        console.log("WebAssembly module loaded successfully in worker");
      } else if (type === "moveDone") {
        console.log("Received move from worker:", payload);

        // Process the result
        const parts = payload.split(";");
        const boardStr = parts[0];
        const winnerStr = parts.length > 1 ? parts[1] : null;

        setBoard(boardStr.split("") as TCell[]);
        setAiThinking(false);
        togglePlayer();

        if (winnerStr) {
          setGameOver(true);
          setWinner(winnerStr === "BLACK" ? "B" : "W");
          setStatusMessage(`Game Over! ${winnerStr === "BLACK" ? "Black" : "White"} wins!`);
        } else {
          setStatusMessage(`Your turn (${currentPlayer === "B" ? "White" : "Black"})`);
        }
      } else if (type === "error") {
        console.error("Worker error:", payload);
        setStatusMessage(`Error: ${payload}`);
        setAiThinking(false);
      }
    };

    worker.postMessage({ type: "init" });
    workerRef.current = worker;

    return () => worker.terminate();
  }, []);

  const handleCellClick = (index: number) => {
    // Ignore clicks when game is over, loading, AI is thinking, or cell is occupied
    if (gameOver || wasmLoading || aiThinking || !workerRef.current || board[index] !== ".") {
      return;
    }

    // Place human move
    const newBoard = [...board];
    newBoard[index] = currentPlayer;
    setBoard(newBoard);

    // Set AI as thinking
    setAiThinking(true);
    setStatusMessage("AI is thinking...");

    // Get AI move using WASM in worker
    const boardString = newBoard.join("");
    const timeLimitMs = 5000;

    togglePlayer();

    workerRef.current.postMessage({
      type: "getBestMove",
      payload: `${boardString};${timeLimitMs}`,
    });
  };

  // Determine board interaction state
  const isBoardInteractive = !wasmLoading && !gameOver && !aiThinking;

  return (
    <div className="flex flex-col items-center gap-4">
      <div className="mb-4 text-center">
        <div className="font-bold text-lg">{statusMessage}</div>

        {/* Player indicator */}
        <div className="flex items-center justify-center gap-2 mt-2">
          <span>Current player:</span>
          <span className={`w-5 h-5`}>
            <UINode position={getUINodePosition(size, 0)} currentPlayer={currentPlayer} takenBy={currentPlayer} hideGrid />
          </span>
        </div>
      </div>
      <div className="bg-gradient-to-br from-stone-300 to-stone-400 rounded-xl overflow-hidden p-6 shadow-[0_4px_8px_rgba(0,0,0,0.6)] border-1 border-stone-600">
        <div className={`grid grid-cols-9 gap-0 w-[450px] h-[450px] relative ${!isBoardInteractive ? "opacity-70" : ""}`}>
          {/* Overlay to prevent clicks when board is not interactive */}
          {!isBoardInteractive && <div className="absolute inset-0 z-10 cursor-not-allowed" />}

          {board.map((cell: TCell, index: number) => (
            <div key={index} className="w-[50px] h-[50px] cursor-pointer" onClick={() => handleCellClick(index)}>
              <UINode position={getUINodePosition(size, index)} currentPlayer={currentPlayer} takenBy={cell} />
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
