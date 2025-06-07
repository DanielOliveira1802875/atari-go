import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import "./index.css";
import { Routes, Route, HashRouter } from "react-router";
import AppContainer from "@/components/AppContainer.tsx";
import { Toaster } from "sonner";
import Board from "@/components/board/Board.tsx";
import Home from "@/components/board/Home.tsx";

createRoot(document.getElementById("root")!).render(
  <StrictMode>
    <Toaster />
    <HashRouter>
      <Routes>
        <Route path={"/"} element={<AppContainer />}>
          <Route path="play" element={<Board />} />
          <Route index element={<Home />} />
        </Route>
      </Routes>
    </HashRouter>
  </StrictMode>,
);
