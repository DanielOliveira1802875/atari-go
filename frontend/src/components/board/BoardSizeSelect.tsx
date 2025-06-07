import { Label } from "@/components/ui/label";
import { RadioGroup, RadioGroupItem } from "@/components/ui/radio-group";
import { usePreferences } from "@/stores/usePreferences.ts";

const BoardSizeSelect = () => {
  const { boardEdge, setBoardEdge } = usePreferences();
  return (
    <RadioGroup
      id={"boardSizeSelect"}
      defaultValue={boardEdge.toString()}
      orientation={"horizontal"}
      className={"w-full flex gap-x-5 gap-y-2 items-baseline"}
      onValueChange={(value) => {
        setBoardEdge(Number(value));
      }}
    >
      <div className="flex items-center space-x-2">
        <RadioGroupItem value="9" id="r2" />
        <Label htmlFor="r2">9x9</Label>
      </div>
      <div className="flex items-center space-x-2">
        <RadioGroupItem value="8" id="r1" />
        <Label htmlFor="r1">8x8</Label>
      </div>
      <div className="flex items-center space-x-2">
        <RadioGroupItem value="7" id="r1" />
        <Label htmlFor="r1">7x7</Label>
      </div>
    </RadioGroup>
  );
};

export default BoardSizeSelect;
