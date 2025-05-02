@echo off
setlocal EnableDelayedExpansion

echo Setting up Emscripten environment...

set EMSDK_PATH=C:\emsdk
call "%EMSDK_PATH%\emsdk_env.bat"
if not exist "..\Frontend\public\wasm" mkdir "..\Frontend\public\wasm"

echo Compiling Atari Go to WebAssembly...

em++ -std=c++17 -O3 -flto -ffast-math -msimd128 ^
  -funroll-loops -ftree-vectorize ^
  wasm_interface.cpp AtariGo.cpp ^
  -o "..\Frontend\public\wasm\atari_go.js" ^
  -s WASM=1 ^
  -s DISABLE_EXCEPTION_CATCHING=0 ^
  -s EXPORTED_FUNCTIONS="['_getBestMove', _getSuccessors]" ^
  -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap', 'lengthBytesUTF8']" ^
  -s ALLOW_MEMORY_GROWTH=1 ^
  -s ENVIRONMENT="web,worker" ^
  -s MALLOC="emmalloc" ^
  -s ASSERTIONS=0 ^
  -s AGGRESSIVE_VARIABLE_ELIMINATION=1 ^
  -s WASM_BIGINT=1 ^
  --closure 1

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed!
) else (
    echo Compilation completed successfully!
    echo Output files: wasm/atari_go.js and wasm/atari_go.wasm
)