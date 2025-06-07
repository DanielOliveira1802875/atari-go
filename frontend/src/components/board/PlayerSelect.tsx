import { Label } from "@/components/ui/label";
import { RadioGroup, RadioGroupItem } from "@/components/ui/radio-group";
import { TPlayer, usePreferences } from "@/stores/usePreferences.ts";

const PlayerSelect = () => {
  const { playerColor, setPlayer } = usePreferences();
  return (
    <RadioGroup
      id={"playerSelect"}
      defaultValue={playerColor}
      orientation={"horizontal"}
      className={"w-full flex gap-x-5 gap-y-2 items-baseline"}
      onValueChange={(value) => {
        setPlayer(value as TPlayer);
      }}
    >
      <div className="flex items-center space-x-2">
        <RadioGroupItem value="W" id="playerW" />
        <Label htmlFor="playerW">Brancas</Label>
      </div>
      <div className="flex items-center space-x-2">
        <RadioGroupItem value="B" id="playerB" />
        <Label htmlFor="playerB">Pretas</Label>
      </div>
    </RadioGroup>
  );
};

export default PlayerSelect;
