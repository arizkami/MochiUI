@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
python scripts/gen_theme.py
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build