let wasmModule: any = null;

// Handle messages from the main thread
self.onmessage = async (event) => {
  const { type, payload } = event.data;

  if (type === 'init') {
    try {
      // Define the Module configuration
      (self as any).Module = {
        locateFile: (path: string) => {
          if (path.endsWith('.wasm')) {
            return '/wasm/atari_go.wasm';
          }
          return path;
        },
        onRuntimeInitialized: () => {
          wasmModule = {
            getBestMove: (self as any).Module.cwrap('getBestMove', 'string', ['string'])
          };
          self.postMessage({ type: 'initialized' });
        },
        print: (text: string) => {
          console.log('WASM stdout:', text);
        },
        printErr: (text: string) => {
          console.error('WASM stderr:', text);
        }
      };

      const response = await fetch('/wasm/atari_go.js');
      const scriptText = await response.text();

      // Execute the script in the worker context
      const scriptBlob = new Blob([scriptText], { type: 'application/javascript' });
      const scriptURL = URL.createObjectURL(scriptBlob);
      await import(scriptURL);
      URL.revokeObjectURL(scriptURL);

    } catch (error: unknown) {
      const errorMessage = error instanceof Error ? error.message : String(error);
      self.postMessage({ type: 'error', payload: errorMessage });
    }
  }

  else if (type === 'getBestMove') {
    if (!wasmModule) {
      self.postMessage({ type: 'error', payload: 'WebAssembly module not initialized' });
      return;
    }

    try {
      const result = wasmModule.getBestMove(payload);
      self.postMessage({ type: 'moveDone', payload: result });
    } catch (error: unknown) {
      const errorMessage = error instanceof Error ? error.message : String(error);
      self.postMessage({ type: 'error', payload: errorMessage });
    }
  }
};