@echo off
setlocal enabledelayedexpansion

rem Usage:
rem   scripts\submodules-init-sync.cmd
rem   scripts\submodules-init-sync.cmd --skip-skia
rem   set SKIP_SKIA=1 && scripts\submodules-init-sync.cmd

where git >nul 2>nul
if errorlevel 1 (
  echo error: git not found on PATH 1>&2
  exit /b 127
)

set "REPO_ROOT=%~dp0.."
pushd "%REPO_ROOT%" >nul

set "SKIP_SKIA=%SKIP_SKIA%"
if /I "%~1"=="--skip-skia" set "SKIP_SKIA=1"

echo ==> Syncing submodule URLs
git submodule sync --recursive
if errorlevel 1 goto :fail

if "%SKIP_SKIA%"=="1" (
  echo ==> Updating submodules ^(skipping external/skia^)
  git submodule update --init --recursive --depth 1 -- ^
    external/yoga ^
    external/lucide ^
    external/rtaudio ^
    external/rtmidi ^
    external/nlohmann-json ^
    external/kissfft ^
    external/libpng ^
    external/webp ^
    external/yaml-cpp ^
    external/libcss
  if errorlevel 1 goto :fail
) else (
  echo ==> Updating submodules ^(recursive^)
  git submodule update --init --recursive
  if errorlevel 1 goto :fail
)

echo ==> Done
popd >nul
exit /b 0

:fail
set "EC=%ERRORLEVEL%"
popd >nul
exit /b %EC%
