import React from "react";
import { usePreferences } from "@/stores/usePreferences.ts";
import { Slider } from "@/components/ui/slider.tsx";

export const MAX_LEVEL = 6;

const LevelSlider = (props: { disabled: boolean }) => {
  const { level, setLevel } = usePreferences();

  return <Slider id="levelSlider" min={1} max={MAX_LEVEL} step={1} value={[level]} onValueChange={(v) => setLevel(v[0])} disabled={props.disabled}/>
};

export default LevelSlider;
