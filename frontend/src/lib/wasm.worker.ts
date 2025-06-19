let wasmModule: any = null;

const WASM_BASE_PATH = "/atari-go/wasm/";

self.onmessage = async (event) => {
  const { type, payload } = event.data;

  if (type === "init") {
    // The payload should include the WASM file name that reflects the size of the board, for example, "atari-go-9x9."
    const fileName = payload?.fileName;
    if (!fileName) {
      self.postMessage({
        type: "error",
        payload: "Initialization error: A 'fileName' must be provided, e.g., 'atari-go-9x9'.",
      });
      return;
    }

    try {
      (self as any).Module = {
        locateFile: (path: string) => {
          return `${WASM_BASE_PATH}${path}`;
        },
        onRuntimeInitialized: () => {
          wasmModule = {
            getBestMove: (self as any).Module.cwrap("getBestMove", "string", ["string"]),
            checkCapture: (self as any).Module.cwrap("checkCapture", "string", ["string"]),
            wasMoveSuicidal: (self as any).Module.cwrap("wasMoveSuicidal", "string", ["string"]),
          };
          // Notify the main thread that we are ready
          self.postMessage({ type: "initialized" });
        },

        // Capture and log stdout from the Wasm module
        print: (text: string) => {
          if (import.meta.env.DEV) console.log("WASM stdout:", text);
        },

        // Capture and log stderr from the Wasm module
        printErr: (text: string) => {
          if (import.meta.env.DEV) console.error("WASM stderr:", text);
        },
      };

      // Construct the full path to the Emscripten JavaScript glue code
      const scriptPath = `${WASM_BASE_PATH}${fileName}.js`;

      // Fetch the script text. This is necessary because module workers can't use importScripts().
      const response = await fetch(scriptPath);
      if (!response.ok) {
        throw new Error(`Failed to fetch script at ${scriptPath}: ${response.statusText}`);
      }
      const scriptText = await response.text();

      // Execute the script in the worker context
      const scriptBlob = new Blob([scriptText], { type: "application/javascript" });
      const scriptURL = URL.createObjectURL(scriptBlob);
      await import(scriptURL);
      URL.revokeObjectURL(scriptURL);
    } catch (error: unknown) {
      const errorMessage = error instanceof Error ? error.message : String(error);
      self.postMessage({ type: "error", payload: errorMessage });
    }
  } else if (type === "getBestMove") {
    if (!wasmModule) {
      self.postMessage({ type: "error", payload: "WebAssembly module not initialized" });
      return;
    }

    try {
      const result = wasmModule.getBestMove(payload);
      self.postMessage({ type: "moveDone", payload: result });
    } catch (error: unknown) {
      const errorMessage = error instanceof Error ? error.message : String(error);
      self.postMessage({ type: "error", payload: errorMessage });
    }
  } else if (type === "checkCapture") {
    if (!wasmModule) {
      self.postMessage({ type: "error", payload: "WebAssembly module not initialized" });
      return;
    }

    try {
      const result = wasmModule.checkCapture(payload);
      self.postMessage({ type: "checkCaptureDone", payload: result });
    } catch (error: unknown) {
      const errorMessage = error instanceof Error ? error.message : String(error);
      self.postMessage({ type: "error", payload: errorMessage });
    }
  } else if (type === "wasMoveSuicidal") {
    if (!wasmModule) {
      self.postMessage({ type: "error", payload: "WebAssembly module not initialized" });
      return;
    }

    try {
      const result = wasmModule.wasMoveSuicidal(payload);
      self.postMessage({ type: "wasMoveSuicidalDone", payload: result });
    } catch (error: unknown) {
      const errorMessage = error instanceof Error ? error.message : String(error);
      self.postMessage({ type: "error", payload: errorMessage });
    }
  }
};
