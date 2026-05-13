# Build, prebuilts, and CI

## Local build (Windows)

From the repo root:

```powershell
.\build.ps1
```

The script:

1. Delegates to `bun run build`.
2. Runs `vcvars64.bat` when needed so MSVC + Ninja see the standard library correctly.
3. Configures with **Ninja** if `build/` is missing, builds JS bundles, then builds CMake targets.

Requirements: **Visual Studio 2022** (or compatible MSVC), **CMake**, **Ninja**, **Bun**.

Useful commands:

- `bun run fetch:deps`
- `bun run build:framework`
- `bun run build:example`
- `bun run build`

## Local configuration (`.env`)

Copy `.env.example` to `.env` to override local build settings without changing tracked files.

Common keys:

- `VS_PATH`: Visual Studio install root or direct `vcvars64.bat` path.
- `USE_PREBUILT`: `ON` by default. Uses Skia/V8 prebuilts.
- `FETCH_PREBUILT`: `ON` by default. Downloads missing prebuilts during CMake configure.
- `PREBUILT_DIR`: defaults to `external/prebuilt`.
- `SKIA_DIR`, `V8_DIR`: default to `external/prebuilt/skia` and `external/prebuilt/v8`.
- `SKIA_URL`, `V8_URL`: optional archive URL overrides.
- `BUILD_DIR`, `CMAKE_BUILD_TYPE`, `CMAKE_GENERATOR`, `CMAKE_PATH`: build tool overrides.
- `FETCH_SUBMODULES`, `BUN_INSTALL_MODULES`: dependency setup toggles for `fetch:deps`.

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
| `SphereKit_Foundation` | `SphereKit.Foundation.dll` |
| `SphereKit_GraphicInterface` | `SphereKit.GraphicInterface.dll` |
| `SphereKit_GraphicComponent` | `SphereKit.GraphicComponent.dll` |
| `SphereKit_JavaScriptEngine` | `SphereKit.JavaScriptEngine.dll` |
| `Sphere_React` | `Sphere.React.dll` |
| `Sphere_Vue` | `SphereKit.Vue.dll` |
| `SphereKit_DirectAudioEngine` | `SphereKit.DirectAudioEngine.dll` (Windows) |
| `ReactUIDemo` | `ReactUIDemo.exe` (example) |
| `VueUIDemo` | `VueUIDemo.exe` (example) |

`MikoUI` is an INTERFACE library (headers + link dependencies only).

## GitHub Actions

Workflow: `.github/workflows/build.yml`

- OS: `windows-latest`
- Caches `external/prebuilt/skia` and `external/prebuilt/v8` keyed on `hashFiles('CMakeLists.txt')`
- Builds the core DLLs plus the supported UI paths: native C++ nodes, JS/React, and JS/Vue
- Uploads **SphereKit-SDK** and **SphereKit-Examples** artifacts

## Related

- [Getting Started](GettingStarted.md)
- [JavaScript and React](JavaScriptAndReact.md)
