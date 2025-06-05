import { Button } from "@/components/ui/button";
import { Card, CardContent, CardDescription, CardFooter, CardHeader, CardTitle } from "@/components/ui/card";
import { useNavigate } from "react-router";
import LevelSlider from "@/components/board/LevelSlider.tsx";
import PlayerSelect from "@/components/board/PlayerSelect.tsx";
import { Form, FormControl, FormField, FormItem, FormLabel } from "@/components/ui/form.tsx";
import { useForm } from "react-hook-form";
import { TooltipProvider } from "@/components/ui/tooltip.tsx";
import { usePreferences } from "@/stores/usePreferences.ts";
import Statistics from "@/components/board/Statistics.tsx";

export default function Home() {
  const navigate = useNavigate();
  const form = useForm();
  const { level } = usePreferences();

  return (
    <TooltipProvider>
      <div className="container mx-auto px-4 py-8 max-w-4xl">
        <div className="flex flex-col items-center gap-8">
          <Card className="w-full shadow-lg border-stone-300 bg-stone-100">
            <CardHeader className="from-stone-200 to-stone-300">
              <CardTitle className="text-2xl font-bold">Atari Go</CardTitle>
              <CardDescription className="text-lg">Campeonato Nacional de Jogos Matemáticos</CardDescription>
            </CardHeader>
            <CardContent className="pt-6">
              <Form {...form}>
                <FormField
                  control={form.control}
                  name="player"
                  render={() => (
                    <FormItem className="mb-4 flex">
                      <FormLabel className={"w-36"}>Jogar com</FormLabel>
                      <FormControl>
                        <PlayerSelect />
                      </FormControl>
                    </FormItem>
                  )}
                />
                <FormField
                  control={form.control}
                  name="level"
                  render={() => (
                    <FormItem className="mb-4 flex items-baseline">
                      <FormLabel className={"w-36"}>{`Dificuldade ${level}`}</FormLabel>
                      <FormControl>
                        <LevelSlider disabled={false} />
                      </FormControl>
                    </FormItem>
                  )}
                />
                <div className="mt-16">
                  <Statistics />
                </div>
              </Form>
            </CardContent>
            <CardFooter className="flex justify-center border-t pt-6">
              <Button size="lg" onClick={() => navigate("/play")} className="bg-stone-800 hover:bg-stone-900">
                Jogar
              </Button>
            </CardFooter>
          </Card>
        </div>
      </div>
    </TooltipProvider>
  );
}
