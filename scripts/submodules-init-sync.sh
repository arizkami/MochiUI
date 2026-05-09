#!/usr/bin/env bash
set -euo pipefail

SKIP_SKIA="${SKIP_SKIA:-0}"
if [[ "${1:-}" == "--skip-skia" ]]; then
  SKIP_SKIA="1"
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

if ! command -v git >/dev/null 2>&1; then
  echo "error: git not found on PATH" >&2
  exit 127
fi

echo "==> Syncing submodule URLs"
git submodule sync --recursive

if [[ "$SKIP_SKIA" == "1" ]]; then
  echo "==> Updating submodules (skipping external/skia)"
  git submodule update --init --recursive --depth 1 -- external/yoga external/lucide external/rtaudio external/rtmidi external/nlohmann-json external/kissfft external/libpng external/webp external/yaml-cpp external/libcss
else
  echo "==> Updating submodules (recursive)"
  git submodule update --init --recursive
fi

echo "==> Done"
