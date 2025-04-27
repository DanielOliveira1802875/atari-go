import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import "./index.css";
import { BrowserRouter, Routes, Route } from "react-router";
import AppContainer from "@/components/AppContainer.tsx";
import { Toaster } from "sonner";
import Home from "@/components/Home.tsx";

createRoot(document.getElementById("root")!).render(
  <StrictMode>
    <Toaster />
    <BrowserRouter basename="/atari-go">
      <Routes>
        <Route path="/" element={<AppContainer />}>
          <Route path="/" element={<Home />} />
        </Route>
      </Routes>
    </BrowserRouter>
  </StrictMode>,
);
