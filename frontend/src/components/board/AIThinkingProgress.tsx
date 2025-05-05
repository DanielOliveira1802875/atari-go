import { useEffect, useState } from "react";
import { Progress } from "@/components/ui/progress.tsx";

type AIThinkingProgressProps = {
  timeLimitMs: number;
  isThinking: boolean;
};

const AIThinkingProgress = ({ timeLimitMs, isThinking }: AIThinkingProgressProps) => {
  const [progress, setProgress] = useState(0);
  const [intervalId, setIntervalId] = useState<number | null>(null);

  const updateInterval = 100;

  useEffect(() => {
    // Clear any existing interval when component unmounts
    return () => {
      if (intervalId !== null) {
        clearInterval(intervalId);
      }
    };
  }, [intervalId]);

  useEffect(() => {
    if (isThinking) {
      // Calculate how much to increment per update
      const incrementAmount = (updateInterval / timeLimitMs) * 100;
      setProgress(incrementAmount);

      // Start progress growing
      const id = setInterval(() => {
        setProgress((prev) => Math.min(100, prev + incrementAmount));
      }, updateInterval);

      setIntervalId(id as unknown as number);
    } else if (intervalId !== null) {
      // Stop when AI is done thinking
      clearInterval(intervalId);
      setIntervalId(null);
      setProgress(0);
    }
  }, [isThinking, timeLimitMs]);

  return (
    <div className={`w-full h-1 transition-all duration-500 ease-in-out ${!isThinking ? "opacity-0 pointer-events-none" : "opacity-100"}`}>
      {isThinking && <Progress value={progress} className="h-1 bg-stone-200" />}
    </div>
  );
};

export default AIThinkingProgress;
