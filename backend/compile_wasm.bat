@echo off
setlocal EnableDelayedExpansion

echo Setting up Emscripten environment...

set EMSDK_PATH=C:\emsdk
call "%EMSDK_PATH%\emsdk_env.bat"
if not exist "..\Frontend\public\wasm" mkdir "..\Frontend\public\wasm"

REM =============== COMPILE 9x9 VERSION ===============
echo Compiling 9x9 Atari Go to WebAssembly...

call em++ -std=c++17 -O3 -flto -ffast-math -msimd128 -funroll-loops -ftree-vectorize ^
  wasm_interface.cpp AtariGo.cpp ^
  -o "..\Frontend\public\wasm\atari_go_9x9.js" ^
  -s WASM=1 ^
  -s DISABLE_EXCEPTION_CATCHING=1 ^
  -s EXPORTED_FUNCTIONS="['_getBestMove', _checkCapture, _wasMoveSuicidal]" ^
  -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap', 'lengthBytesUTF8']" ^
  -s ALLOW_MEMORY_GROWTH=1 ^
  -s ENVIRONMENT="web,worker" ^
  -s MALLOC="emmalloc" ^
  -s ASSERTIONS=0 ^
  -s AGGRESSIVE_VARIABLE_ELIMINATION=1 ^
  -s WASM_BIGINT=1 ^
  --closure 1

if %ERRORLEVEL% NEQ 0 (
    echo 9x9 Compilation failed!
    exit /b 1
) else (
    echo 9x9 Compilation completed successfully!
    echo Output files: wasm/atari_go_9x9.js and wasm/atari_go_9x9.wasm
)

REM =============== COMPILE 8x8 VERSION ===============
echo Compiling 8x8 Atari Go to WebAssembly...

call em++ -std=c++17 -O3 -flto -ffast-math -msimd128 -funroll-loops -ftree-vectorize ^
  -DBOARD_EDGE_OVERRIDE=8 ^
  wasm_interface.cpp AtariGo.cpp ^
  -o "..\Frontend\public\wasm\atari_go_8x8.js" ^
  -s WASM=1 ^
  -s DISABLE_EXCEPTION_CATCHING=1 ^
  -s EXPORTED_FUNCTIONS="['_getBestMove', _checkCapture, _wasMoveSuicidal]" ^
  -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap', 'lengthBytesUTF8']" ^
  -s ALLOW_MEMORY_GROWTH=1 ^
  -s ENVIRONMENT="web,worker" ^
  -s MALLOC="emmalloc" ^
  -s ASSERTIONS=0 ^
  -s AGGRESSIVE_VARIABLE_ELIMINATION=1 ^
  -s WASM_BIGINT=1 ^
  --closure 1

if %ERRORLEVEL% NEQ 0 (
    echo 8x8 Compilation failed!
    exit /b 1
) else (
    echo 8x8 Compilation completed successfully!
    echo Output files: wasm/atari_go_8x8.js and wasm/atari_go_8x8.wasm
)

REM =============== COMPILE 7x7 VERSION ===============
echo Compiling 7x7 Atari Go to WebAssembly...

call em++ -std=c++17 -O3 -flto -ffast-math -msimd128 -funroll-loops -ftree-vectorize ^
  -DBOARD_EDGE_OVERRIDE=7 ^
  wasm_interface.cpp AtariGo.cpp ^
  -o "..\Frontend\public\wasm\atari_go_7x7.js" ^
  -s WASM=1 ^
  -s DISABLE_EXCEPTION_CATCHING=1 ^
  -s EXPORTED_FUNCTIONS="['_getBestMove', _checkCapture, _wasMoveSuicidal]" ^
  -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap', 'lengthBytesUTF8']" ^
  -s ALLOW_MEMORY_GROWTH=1 ^
  -s ENVIRONMENT="web,worker" ^
  -s MALLOC="emmalloc" ^
  -s ASSERTIONS=0 ^
  -s AGGRESSIVE_VARIABLE_ELIMINATION=1 ^
  -s WASM_BIGINT=1 ^
  --closure 1

if %ERRORLEVEL% NEQ 0 (
    echo 7x7 Compilation failed!
    exit /b 1
) else (
    echo 7x7 Compilation completed successfully!
    echo Output files: wasm/atari_go_7x7.js and wasm/atari_go_7x7.wasm
)

echo.
echo All builds finished.