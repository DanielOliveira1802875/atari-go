import React from "react";
import { usePreferences } from "@/stores/usePreferences.ts";
import { Slider } from "@/components/ui/slider.tsx";

const LevelSlider = (props: { disabled: boolean }) => {
  const { level, ...gamePrefs } = usePreferences();

  return (
    <div className="w-full max-w-md flex flex-col items-center gap-2">
      <span className="font-semibold">Difficulty: Level {level}</span>
      <Slider min={1} max={6} step={1} value={[level]} onValueChange={(v) => gamePrefs.setLevel(v[0])} disabled={props.disabled} className="w-full" />
    </div>
  );
};

export default LevelSlider;
