#!/usr/bin/env pwsh
[CmdletBinding()]
param(
  [switch]$SkipSkia
)

$ErrorActionPreference = 'Stop'

function Assert-Git {
  $git = Get-Command git -ErrorAction SilentlyContinue
  if (-not $git) { throw "git not found on PATH" }
}

Assert-Git

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
Set-Location $repoRoot

if ($env:SKIP_SKIA -eq '1') { $SkipSkia = $true }

Write-Host "==> Syncing submodule URLs"
git submodule sync --recursive

if ($SkipSkia) {
  Write-Host "==> Updating submodules (skipping external/skia)"
  git submodule update --init --recursive --depth 1 -- `
    external/yoga `
    external/lucide `
    external/rtaudio `
    external/rtmidi `
    external/nlohmann-json `
    external/kissfft `
    external/libpng `
    external/webp `
    external/yaml-cpp `
    external/libcss
} else {
  Write-Host "==> Updating submodules (recursive)"
  git submodule update --init --recursive
}

Write-Host "==> Done"
