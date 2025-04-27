import { cn } from "@/lib/utils.ts";
import {  Outlet } from "react-router";

function AppContainer(props: { className?: string }) {

  return (
    <div className="flex flex-col h-screen w-screen bg-bgColor text-textColor bg-gradient-to-br from-stone-200 to-stone-300">
      <div className={cn("container px-4 m-auto", props.className)}>
        <Outlet />
      </div>
    </div>
  );
}

export default AppContainer;
