import { TCell, TPlayer } from "@/stores/usePreferences.ts";
import { TUINodePosition } from "@/lib/Helpers.ts";

interface UINodeProps {
  position: TUINodePosition;
  currentPlayer: TPlayer;
  takenBy: TCell;
  hideGrid?: boolean;
  isLastMove?: boolean;
  isCaptured?: boolean;
}

const BoardCell = (props: UINodeProps) => {
  const { position, currentPlayer, takenBy } = props;

  let div1Border: string;
  if (position === "upperLeft") div1Border = "";
  else if (["topEdge", "upperRight"].includes(position)) div1Border = "border-b-2";
  else if (["lowerLeft", "leftEdge"].includes(position)) div1Border = "border-r-2";
  else div1Border = "border-r-2 border-b-2";

  let div2Border: string;
  if (["rightEdge", "upperRight", "lowerRight"].includes(position)) div2Border = "";
  else div2Border = "border-b-2";

  let div3Border: string;
  if (["bottomEdge", "lowerRight", "lowerLeft"].includes(position)) div3Border = "";
  else div3Border = "border-r-2";

  let stoneStyle = "w-[95%] h-[95%] rounded-full transition-all duration-100 ease-in-out";
  if (takenBy != ".") stoneStyle += " opacity-100";
  else stoneStyle += " opacity-0 hover:opacity-50";

  if (props.isLastMove) {
    stoneStyle += " border-2 border-blue-500";
  } else if (props.isCaptured) {
    stoneStyle += " border-2 border-rose-600";
  }

  // Adding stone styling with shadows and highlights
  if (takenBy === "B" || (currentPlayer === "B" && takenBy === ".")) {
    stoneStyle += " bg-gradient-to-br from-stone-500 to-black";
    stoneStyle += " shadow-[0_2px_3px_rgba(0,0,0,0.6),inset_0_-2px_5px_rgba(0,0,0,0.8),inset_0_2px_5px_rgba(255,255,255,0.3)]";
  } else if (takenBy === "W" || (currentPlayer === "W" && takenBy === ".")) {
    stoneStyle += " bg-gradient-to-br from-stone-100 to-stone-500";
    stoneStyle += " shadow-[0_2px_3px_rgba(0,0,0,0.4),inset_0_-2px_5px_rgba(0,0,0,0.1),inset_0_2px_5px_rgba(255,255,255,0.7)]";
  }

  return (
    <div className="h-full w-full flex flex-col relative">
      <div className="absolute z-10 size-full flex items-center justify-center">
        <div className={stoneStyle}></div>
      </div>
      {!props.hideGrid && (
        <>
          <div className="absolute w-[2px] h-[2px] bg-primary top-1/2 left-1/2 -translate-x-[1px] -translate-y-[2px]" />
          <div className="flex-1 w-full flex items-center justify-center">
            <div className={`flex-1 h-full border-stone-800 ${div1Border}`} />
            <div className={`flex-1 h-full border-stone-800 ${div2Border}`} />
          </div>
          <div className="flex-1 w-full flex items-center justify-center">
            <div className={`flex-1 h-full border-stone-800 ${div3Border}`} />
            <div className="flex-1 h-full" />
          </div>
        </>
      )}
    </div>
  );
};

export default BoardCell;
