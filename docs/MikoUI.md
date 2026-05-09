# MikoUI (minimal framework)

MikoUI is a **header-only convenience layer** that wires `Application`, `JavaScriptEngine`, `RenderReactApp`, and `Win32Window` for quick experiments.

## Location

- `framework/MikoUI/include/MikoUI.hpp`
- CMake: `add_subdirectory(framework/MikoUI)` defines an **INTERFACE** target `MikoUI` linking:
  - `SphereKit_GraphicComponent`
  - `SphereKit_JavaScriptEngine`
  - `Sphere_React`

## Usage

```cpp
#include <MikoUI.hpp>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    SphereUI::MikoUI::AppConfig cfg;
    cfg.title = "My App";

    std::string js = R"(
      const r = SphereUI.createNode("column");
      SphereUI.setStyle(r, "backgroundColor", "#1a1a2e");
      SphereUI.setRoot(r);
    )";

    return SphereUI::MikoUI::Run(cfg, js, "inline.js");
}
```

Pass a full `react-reconciler` bundle from `modules/reactui` once built.

## Related

- [JavaScript and React](JavaScriptAndReact.md)
- [Getting Started](GettingStarted.md) for imperative UI without JS.
