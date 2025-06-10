import React from "react";
import { usePreferences } from "@/stores/usePreferences.ts";
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table.tsx";
import BoardCell from "@/components/board/BoardCell.tsx";

const Statistics = () => {
  const { statistics, playerColor, level } = usePreferences();

  return (
    <Table className="text-center text-xs ">
      <TableHeader>
        <TableRow className="border-bottom-2 border-stone-500">
          <TableHead className="text-center">Vitórias - Derrotas</TableHead>
          {Object.keys(statistics.B).map((key) => (
            <TableHead key={key} className="text-center">
              Nível {key}
            </TableHead>
          ))}
        </TableRow>
      </TableHeader>
      <TableBody>
        <TableRow className={`border-bottom-2 border-stone-500`}>
          <TableCell className={"text-center flex justify-center items-center"}>
            <div className={"size-3"}>
              <BoardCell position={"middle"} hideGrid currentPlayer={"B"} takenBy={"B"} />
            </div>
          </TableCell>
          {Object.values(statistics.B).map((value, index) => (
            <TableCell key={index} className={`text-center ${level === index + 1 && playerColor === "B" ? "bg-gray-300 rounded-xs" : ""}`}>
              <span className={"text-green-700"}>{value.wins}</span> - <span className={"text-red-700"}>{value.losses}</span>
            </TableCell>
          ))}
        </TableRow>
        <TableRow className={`border-bottom-2 border-stone-500`}>
          <TableCell className={"text-center flex justify-center items-center"}>
            <div className={"size-3"}>
              <BoardCell position={"middle"} hideGrid currentPlayer={"W"} takenBy={"W"} />
            </div>
          </TableCell>
          {Object.values(statistics.W).map((value, index) => (
            <TableCell key={index} className={`text-center ${level === index + 1 && playerColor === "W" ? "bg-gray-300 rounded-xs" : ""}`}>
              <span className={"text-green-700"}>{value.wins}</span> - <span className={"text-red-700"}>{value.losses}</span>
            </TableCell>
          ))}
        </TableRow>
      </TableBody>
    </Table>
  );
};

export default Statistics;
