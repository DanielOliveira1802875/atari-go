import { cn } from "@/lib/utils.ts";
import { Outlet } from "react-router";
import { useEffect, useState } from "react";

function AppContainer(props: { className?: string }) {
  const [footerHtml, setFooterHtml] = useState("");

  useEffect(() => {
    const fetchNav = async () => {
      try {
        const response = await fetch("https://danieloliveira1802875.github.io/cnjm-footer/");
        if (!response.ok) {
          throw new Error(`HTTP error! status: ${response.status}`);
        }
        const html = await response.text();

        setFooterHtml(html);
      } catch (e) {
        console.error("Failed to fetch footer HTML:", e);
      }
    };

    fetchNav();
  }, [setFooterHtml]);

  return (
    <div className="flex flex-col h-screen w-screen bg-bgColor text-textColor bg-gradient-to-br from-zinc-200 to-zinc-300 justify-between">
      <div className={cn("container px-4 m-auto", props.className)}>
        <Outlet />
      </div>
      <footer className={"flex justify-center w-screen border-t border-gray-500 flex-initial p-2 bg-zinc-300"}>
        <div dangerouslySetInnerHTML={{ __html: footerHtml }} className="w-full text-center text-sm" />
      </footer>
    </div>
  );
}

export default AppContainer;
