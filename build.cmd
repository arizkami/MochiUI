@echo off
cd %~dp0
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake -S . -G "Ninja" -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j%NUMBER_OF_PROCESSORS%
