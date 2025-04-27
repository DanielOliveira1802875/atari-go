export type TUINodePosition = "upperLeft" | "upperRight" | "lowerLeft" | "lowerRight" | "leftEdge" | "rightEdge" | "topEdge" | "bottomEdge" | "middle";

export const getUINodePosition = (boardSize:number, index:number) : TUINodePosition => {
  if ( index === 0 ) return "upperLeft";
  if ( index === boardSize - 1 ) return "upperRight";
  if ( index === boardSize * ( boardSize - 1 ) ) return "lowerLeft";
  if ( index === boardSize ** 2 - 1 ) return "lowerRight";
  if ( index % boardSize === 0 ) return "leftEdge";
  if ( index % boardSize === boardSize - 1 ) return "rightEdge";
  if ( index < boardSize ) return "topEdge";
  if ( index >= boardSize ** 2 - boardSize ) return "bottomEdge";
  return "middle";
}