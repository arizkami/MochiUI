# MikoUI (minimal framework)

MikoUI is a **header-only convenience layer** that wires `Application`, `JavaScriptEngine`, `RenderReactApp`, and `Win32Window` for quick experiments.

## Location

- `framework/MikoUI/include/MikoUI.hpp`
- CMake: `add_subdirectory(framework/MikoUI)` defines an **INTERFACE** target `MikoUI` linking:
  - `AureliaKit_GraphicComponent`
  - `AureliaKit_JavaScriptEngine`
  - `Aurelia_React`

## Usage

```cpp
#include <MikoUI.hpp>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    AureliaUI::MikoUI::AppConfig cfg;
    cfg.title = "My App";

    std::string js = R"(
      const r = AureliaUI.createNode("column");
      AureliaUI.setStyle(r, "backgroundColor", "#1a1a2e");
      AureliaUI.setRoot(r);
    )";

    return AureliaUI::MikoUI::Run(cfg, js, "inline.js");
}
```

Pass a full `react-reconciler` bundle from `modules/reactui` once built.

## Related

- [JavaScript and React](JavaScriptAndReact.md)
- [Getting Started](GettingStarted.md) for imperative UI without JS.
