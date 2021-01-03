@echo off

set BUILD_CONFIG=%1
set BUILD_DIR=%2

IF NOT DEFINED BUILD_CONFIG (SET BUILD_CONFIG="Debug") 
IF NOT DEFINED BUILD_DIR (SET BUILD_DIR="build") 

cmake -G "Visual Studio 16 2019" -A x64 -S . -B %BUILD_DIR%

cmake --build %BUILD_DIR% --target HeapDebugger --config %BUILD_CONFIG%
