import { Button } from "@/components/ui/button";
import { Card, CardContent, CardDescription, CardFooter, CardHeader, CardTitle } from "@/components/ui/card";
import { Link } from "react-router";
import LevelSlider from "@/components/board/LevelSlider.tsx";
import PlayerSelect from "@/components/board/PlayerSelect.tsx";
import { Form, FormControl, FormField, FormItem, FormLabel } from "@/components/ui/form.tsx";
import { useForm } from "react-hook-form";
import { TooltipProvider } from "@/components/ui/tooltip.tsx";
import { usePreferences } from "@/stores/usePreferences.ts";
import BoardSizeSelect from "@/components/board/BoardSizeSelect.tsx";
import { Separator } from "../ui/separator";
import BoardCell from "@/components/board/BoardCell.tsx";

export default function Home() {
  const form = useForm();
  const { statistics, playerColor, level, boardEdge } = usePreferences();
  const stats = statistics[boardEdge][playerColor][level];

  return (
    <TooltipProvider>
      <div className="container mx-auto px-4 py-8 max-w-4xl bg-debugging-red">
        <div className="flex flex-col items-center gap-8">
          <Card className="w-full shadow-lg border-stone-300 bg-stone-100">
            <CardHeader className="from-stone-200 to-stone-300">
              <div className="flex gap-3 align-middle items-center">
                <div className="flex items-center bg-gradient-to-br from-zinc-200 to-zinc-300 rounded-[4px] p-[3px] border border-zinc-500 mb-0.5">
                  <div>
                    <div className={"size-3"}>
                      <BoardCell position={"upperLeft"} currentPlayer={"B"} takenBy={"B"} hideGrid />
                    </div>
                    <div className={"size-3"}>
                      <BoardCell position={"lowerLeft"} currentPlayer={"W"} takenBy={"W"} hideGrid />
                    </div>
                  </div>
                  <div>
                    <div className={"size-3"}>
                      <BoardCell position={"upperRight"} currentPlayer={"W"} takenBy={"W"} hideGrid />
                    </div>
                    <div className={"size-3"}>
                      <BoardCell position={"lowerRight"} currentPlayer={"B"} takenBy={"B"} hideGrid />
                    </div>
                  </div>
                </div>
                <CardTitle className="text-4xl font-bold mb-1">Atari Go</CardTitle>
              </div>
              <CardDescription className="text-lg">Campeonato Nacional de Jogos Matemáticos</CardDescription>
            </CardHeader>
            <CardContent>
              <Separator className="mb-6" />
              <CardDescription className="mb-4 text-md font-semibold">Configuração</CardDescription>
              <Form {...form}>
                <FormField
                  control={form.control}
                  name="player"
                  render={() => (
                    <FormItem className="mb-4 flex">
                      <FormLabel className={"w-30 text-xs md:text-sm"}>Jogar com</FormLabel>
                      <FormControl className="flex-1">
                        <PlayerSelect />
                      </FormControl>
                    </FormItem>
                  )}
                />
                <FormField
                  control={form.control}
                  name="boardSize"
                  render={() => (
                    <FormItem className="mb-4 flex">
                      <FormLabel className={"w-30 text-xs md:text-sm"}>Tabuleiro</FormLabel>
                      <FormControl className="flex-1">
                        <BoardSizeSelect />
                      </FormControl>
                    </FormItem>
                  )}
                />
                <FormField
                  control={form.control}
                  name="level"
                  render={() => (
                    <FormItem className="mb-4 flex items-baseline">
                      <FormLabel className={"w-30 text-xs md:text-sm"}>{`Dificuldade ${level}`}</FormLabel>
                      <FormControl className="w-full">
                        <LevelSlider disabled={false} />
                      </FormControl>
                    </FormItem>
                  )}
                />
                <Separator className="my-6" />
                <CardDescription className="mb-4 text-md font-semibold">Resultado da configuração atual</CardDescription>
                <FormField
                  control={form.control}
                  name="wins"
                  render={() => (
                    <FormItem className="flex items-baseline">
                      <FormLabel className={"w-30 text-xs md:text-sm"}>{`Vitórias`}</FormLabel>
                      <FormControl className="w-full">
                        <span className={"text-xs md:text-sm"}>{stats.wins}</span>
                      </FormControl>
                    </FormItem>
                  )}
                />
                <FormField
                  control={form.control}
                  name="losses"
                  render={() => (
                    <FormItem className="flex items-baseline">
                      <FormLabel className={"w-30 text-xs md:text-sm"}>{`Derrotas`}</FormLabel>
                      <FormControl className="w-full">
                        <span className={"text-xs md:text-sm"}>{stats.losses}</span>
                      </FormControl>
                    </FormItem>
                  )}
                />
              </Form>
            </CardContent>
            <CardFooter className="flex justify-center border-t pt-6">
              <Button size="lg" asChild className="bg-stone-800 hover:bg-stone-900 w-full sm:w-auto">
                <Link to="/play">Jogar</Link>
              </Button>
            </CardFooter>
          </Card>
        </div>
      </div>
    </TooltipProvider>
  );
}
