# Build, prebuilts, and CI

## Local build (Windows)

From the repo root:

```powershell
.\build.ps1
```

The script:

1. Runs `vcvars64.bat` so MSVC + Ninja see the standard library correctly.
2. Runs `python scripts/gen_theme.py` (regenerates `include/gui/Theme.hpp` when themes change).
3. Configures with **Ninja** if `build/` is missing, then builds.

Requirements: **Visual Studio 2022** (or compatible MSVC), **CMake**, **Ninja**, **Python 3**.

## CMake prebuilts

### Skia

Downloaded on first configure if `external/prebuilt/skia/lib/skia.lib` (Windows) is missing. URL is defined in the root `CMakeLists.txt`.

### V8 (JavaScript engine)

Expected layout after download or manual copy:

- `external/prebuilt/v8/include/v8.h`
- `external/prebuilt/v8/lib/v8_monolith.lib`

CMake sets `/Zc:__cplusplus` on MSVC so V8’s C++20 check matches `-std:c++20`.

If an archive was extracted one level too high (`external/prebuilt/include`, `lib`, …), the CMake logic can move those folders under `external/prebuilt/v8` once.

## Targets of interest

| Target | Artifact |
|--------|----------|
| `AureliaKit_Foundation` | `AureliaKit.Foundation.dll` |
| `AureliaKit_GraphicInterface` | `AureliaKit.GraphicInterface.dll` |
| `AureliaKit_GraphicComponent` | `AureliaKit.GraphicComponent.dll` |
| `AureliaKit_DSL` | `AureliaKit.DSL.dll` |
| `AureliaKit_JavaScriptEngine` | `AureliaKit.JavaScriptEngine.dll` |
| `Aurelia_React` | `Aurelia.React.dll` |
| `AureliaKit_DirectAudioEngine` | `AureliaKit.DirectAudioEngine.dll` (Windows) |
| `DSLDemo` | `DSLDemo.exe` (example) |

`MikoUI` is an INTERFACE library (headers + link dependencies only).

## GitHub Actions

Workflow: `.github/workflows/build.yml`

- OS: `windows-latest`
- Caches `external/prebuilt/skia` and `external/prebuilt/v8` keyed on `hashFiles('CMakeLists.txt')`
- Builds the core DLLs, DSL, JS/React, and examples including `DSLDemo`
- Uploads **AureliaKit-SDK** and **AureliaKit-Examples** artifacts

## Related

- [Getting Started](GettingStarted.md)
- [JavaScript and React](JavaScriptAndReact.md)
