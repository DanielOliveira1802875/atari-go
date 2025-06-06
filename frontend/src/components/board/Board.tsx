import { useCallback, useEffect, useRef, useState } from "react";
import { Button } from "@/components/ui/button";
import BoardCell from "@/components/board/BoardCell.tsx";
import { levelConfigs, TCell, TPlayer, usePreferences } from "@/stores/usePreferences.ts";
import { getUINodePosition } from "@/lib/Helpers.ts";
import { useNavigate } from "react-router";
import StatusMessage from "@/components/board/StatusMessage.tsx";
import AIThinkingProgress from "@/components/board/AIThinkingProgress.tsx";
import { ArrowLeftToLine, ArrowRightToLine } from "lucide-react";

const findLastMoveIndex = (oldB: TCell[] | undefined, newB: TCell[]): number | null => {
  if (!oldB) return null; // For the very first board in history
  for (let i = 0; i < newB.length; i++) {
    if (oldB[i] === "." && (newB[i] === "B" || newB[i] === "W")) return i;
  }
  return null;
};

const getEmptyBoard = (size: number): TCell[] => {
  const b = Array(size).fill(".") as TCell[];
  /*b[3 * 9 + 4] = "W";
  b[3 * 9 + 5] = "B";
  b[4 * 9 + 5] = "W";
  b[4 * 9 + 4] = "B";*/
  return b;
};

export default function Board() {
  const { playerColor, level, boardEdge, addWin, addLoss } = usePreferences();

  const BOARD_SIZE = boardEdge * boardEdge;

  const initialBoardState = getEmptyBoard(BOARD_SIZE);
  const [board, setBoard] = useState<TCell[]>(initialBoardState);
  const [boardHistory, setBoardHistory] = useState<TCell[][]>([initialBoardState]);

  const depthLimit = levelConfigs[level].depth;
  const timeLimitMs = levelConfigs[level].time;

  const [currentPlayer, setCurrentPlayer] = useState<TPlayer>("B");
  const [gameOver, setGameOver] = useState(false);
  const [statusMessage, setStatusMessage] = useState(<span>A carregar o motor de jogo...</span>);
  const [lastMoveIndex, setLastMoveIndex] = useState<number | null>(null);
  const [wasmLoading, setWasmLoading] = useState(true);
  const [aiThinking, setAiThinking] = useState(false);
  const capturedStonesIndexes = useRef<number[]>([]);
  const finalWinner = useRef<TPlayer | null>(null); // To store the winner for review status
  const navigate = useNavigate();

  const workerRef = useRef<Worker | null>(null);

  const [isReviewMode, setIsReviewMode] = useState(false);
  const [reviewBoardIndex, setReviewBoardIndex] = useState(0);

  const resetGameState = useCallback(() => {
    capturedStonesIndexes.current = [];
    finalWinner.current = null;
    setLastMoveIndex(null);

    const newEmptyBoard = getEmptyBoard(BOARD_SIZE);
    setBoard(newEmptyBoard);
    setBoardHistory([newEmptyBoard]);

    setGameOver(false);
    setCurrentPlayer("B");
    setAiThinking(false);
    setIsReviewMode(false);
    setReviewBoardIndex(0);
  }, [BOARD_SIZE]);

  const togglePlayer = useCallback(() => {
    setCurrentPlayer((prev) => (prev === "B" ? "W" : "B"));
  }, []);

  const checkWinner = useCallback((currentBoardState: TCell[]) => {
    workerRef.current?.postMessage({ type: "checkCapture", payload: `${currentBoardState.join("")}` });
  }, []);

  useEffect(() => {
    const worker = new Worker(new URL("../../lib/wasm.worker.ts", import.meta.url), { type: "module" });
    worker.postMessage({
      type: "init",
      payload: {
        fileName: `atari_go_${boardEdge}x${boardEdge}`,
      },
    });
    worker.onmessage = ({ data }) => {
      const { type, payload } = data;
      if (type === "error") {
        console.error("Worker error:", payload);
        return;
      }
      if (type === "initialized") {
        setWasmLoading(false);
      }
      if (type === "moveDone") {
        const [boardStr] = payload.split(";");
        const newBoardFromAI = boardStr.split("") as TCell[];

        setBoard((prevBoard) => {
          setLastMoveIndex(findLastMoveIndex(prevBoard, newBoardFromAI));
          return newBoardFromAI;
        });
        setBoardHistory((prevHistory) => [...prevHistory, newBoardFromAI]);

        checkWinner(newBoardFromAI);
      }
      if (type === "checkCaptureDone") {
        if (payload === "") {
          setAiThinking(false);
          if (!gameOver) {
            // Only toggle player if game is not over
            togglePlayer();
          }
          return;
        }
        const [capturedIndexesStr, winnerStr] = payload.split(";");
        capturedStonesIndexes.current = capturedIndexesStr.split(",").map((index: string) => parseInt(index, 10));
        setGameOver(true);
        const winner = (winnerStr === "BLACK" ? "B" : "W") as TPlayer;
        finalWinner.current = winner;
        setStatusMessage(<span>{`A partida terminou, as ${winner === "B" ? "pretas" : "brancas"} venceram!`}</span>);
        setAiThinking(false);
        if (winner === playerColor) addWin();
        else addLoss();
      }
    };
    worker.postMessage({ type: "init" });
    workerRef.current = worker;
    return () => worker.terminate();
  }, [checkWinner, togglePlayer]);

  useEffect(() => {
    if (isReviewMode) return;
    if (gameOver) return;

    if (!wasmLoading && currentPlayer !== playerColor && !aiThinking) {
      setAiThinking(true);
      setStatusMessage(<span>Aguarde pela jogada da IA...</span>);
      workerRef.current?.postMessage({ type: "getBestMove", payload: `${board.join("")};${timeLimitMs};${depthLimit}` });
    } else if (!wasmLoading && currentPlayer === playerColor && !aiThinking) {
      setStatusMessage(<span>Sua vez</span>);
    } else if (wasmLoading) {
      setStatusMessage(<span>A carregar o motor de jogo...</span>);
    }
  }, [board, currentPlayer, playerColor, aiThinking, wasmLoading, gameOver, timeLimitMs, depthLimit, isReviewMode]);

  const handleCellClick = (idx: number) => {
    if (isReviewMode || !playerColor || gameOver || wasmLoading || aiThinking || board[idx] !== "." || currentPlayer !== playerColor) return;

    const newPlayerBoard = [...board];
    newPlayerBoard[idx] = playerColor;

    setLastMoveIndex(idx);
    setBoard(newPlayerBoard);
    setBoardHistory((prevHistory) => [...prevHistory, newPlayerBoard]);

    checkWinner(newPlayerBoard);
  };

  const startReview = () => {
    if (boardHistory.length <= 1) return;
    setIsReviewMode(true);
    setReviewBoardIndex(0);
    setBoard(boardHistory[0]);
    setLastMoveIndex(null);
    setStatusMessage(<span>Revisão da partida</span>);
  };

  const exitReview = () => {
    setIsReviewMode(false);
    setReviewBoardIndex(0);
    setBoard(boardHistory[boardHistory.length - 1]);
    setLastMoveIndex(findLastMoveIndex(boardHistory[boardHistory.length - 2], boardHistory[boardHistory.length - 1]));
    if (gameOver && finalWinner.current) {
      setStatusMessage(<span>{`A partida terminou, as ${finalWinner.current === "B" ? "pretas" : "brancas"} venceram!`}</span>);
    } else if (!wasmLoading) {
      setStatusMessage(<span>Sua vez</span>);
    }
  };

  const navigateReview = (direction: "next" | "prev") => {
    let newIndex = reviewBoardIndex;
    if (direction === "next" && reviewBoardIndex < boardHistory.length - 1) {
      newIndex++;
    } else if (direction === "prev" && reviewBoardIndex > 0) {
      newIndex--;
    } else {
      return;
    }

    setReviewBoardIndex(newIndex);
    const currentReviewBoardState = boardHistory[newIndex];
    const prevReviewBoardState = newIndex > 0 ? boardHistory[newIndex - 1] : undefined;
    setBoard(currentReviewBoardState);
    setLastMoveIndex(findLastMoveIndex(prevReviewBoardState, currentReviewBoardState));
    const moveNumber = newIndex; // 0 is initial, 1 is first move
    setStatusMessage(<span>{`Revisão: Jogada ${moveNumber} de ${boardHistory.length - 1}`}</span>);
  };

  const isBoardInteractive = !isReviewMode && !wasmLoading && !gameOver && !aiThinking && !!playerColor && currentPlayer === playerColor;

  const displayBoard = isReviewMode ? boardHistory[reviewBoardIndex] : board;

  let styleGridCols = `grid-cols-9`;
  if (boardEdge === 7) styleGridCols = `grid-cols-7`;
  let styleBigCellSize = "size-[28px] sm:size-[49px]";
  if (boardEdge === 7) styleBigCellSize = "size-[36px] sm:size-[63px]";

  return (
    <div className="flex flex-col items-center gap-6">
      <div className="text-center">
        <StatusMessage>{statusMessage}</StatusMessage>
      </div>

      <div className="w-full max-w-md">
        <AIThinkingProgress timeLimitMs={timeLimitMs} isThinking={aiThinking && !gameOver} />
      </div>

      <div className="bg-gradient-to-br from-stone-300 to-stone-400 rounded-xl p-4 sm:p-6 shadow-lg border relative border-stone-400 overflow-hidden">
        <div
          className={`transition-all duration-500 ease-in-out absolute top-0 bottom-0 left-0 right-0 z-20 bg-zinc-900 opacity-0 ${
            isBoardInteractive ? "pointer-events-none" : "pointer-events-auto" + !isBoardInteractive && !isReviewMode ? " opacity-40" : ""
          }`}
        />
        <div className={`grid ${styleGridCols} size-[252px] relative sm:size-[441px]`}>
          {displayBoard.map((cell, idx) => (
            <div key={idx} className={`${styleBigCellSize} cursor-pointer`} onClick={() => handleCellClick(idx)}>
              {/*{idx}*/}
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
        {!isReviewMode && (
          <>
            <Button size="lg" onClick={() => navigate("/")} className="bg-stone-800 hover:bg-stone-950">
              Voltar
            </Button>
            <Button disabled={wasmLoading || aiThinking} size="lg" onClick={() => resetGameState()} className="bg-stone-800 hover:bg-stone-950 ml-4">
              Reiniciar
            </Button>
            {gameOver && boardHistory.length > 1 && (
              <Button size="lg" onClick={startReview} className="bg-blue-800 hover:bg-blue-950 ml-4">
                Rever Partida
              </Button>
            )}
          </>
        )}
        {isReviewMode && (
          <div className="flex items-center justify-between gap-2">
            <Button size="lg" onClick={exitReview} className="bg-stone-800 hover:bg-stone-950">
              Voltar
            </Button>
            <Button size="lg" onClick={() => resetGameState()} className="bg-stone-800 hover:bg-stone-950 ml-4">
              Reiniciar
            </Button>
            <Button size="icon" onClick={() => navigateReview("prev")} disabled={reviewBoardIndex === 0} className="bg-stone-800 hover:bg-stone-950 ml-4 cursor-pointer">
              <ArrowLeftToLine className="w-4 h-4" />
            </Button>
            <Button
              size="icon"
              onClick={() => navigateReview("next")}
              disabled={reviewBoardIndex === boardHistory.length - 1}
              className="bg-stone-800 hover:bg-stone-950 cursor-pointer"
            >
              <ArrowRightToLine className="w-4 h-4" />
            </Button>
          </div>
        )}
      </div>
    </div>
  );
}
