# JavaScript engine and Aurelia.React

Windows builds include a V8-based embed and a small bridge DLL for driving `FlexNode` trees from JavaScript (including a React reconciler running in JS).

## Deliverables

| CMake target | Output name | Role |
|--------------|-------------|------|
| `AureliaKit_JavaScriptEngine` | `AureliaKit.JavaScriptEngine.dll` | V8 isolate, `eval()`, native `AureliaUI` bindings |
| `Aurelia_React` | `Aurelia.React.dll` | Thin API: `RenderReactApp(...)` |

Headers:

- `include/AUKJavaScriptEngine.hpp` — `JavaScriptEngine`
- `include/AureliaReact.hpp` — `RenderReactApp`

## JavaScriptEngine

```cpp
AureliaUI::JavaScriptEngine engine;
engine.init();
engine.installAureliaUIGlobal();  // defines globalThis.AureliaUI
bool ok = engine.eval(script, "my.js");
// on failure: engine.lastError()
```

Shut down when done: `engine.shutdown()`.

## Native binding: `globalThis.AureliaUI`

Numeric handles identify `FlexNode` instances in a per-isolate registry.

| Method | Description |
|--------|-------------|
| `createNode(type: string) → number` | Creates a node. Types: `column`, `row`, `div` (column flex), or other (generic `FlexNode::Create()`) |
| `appendChild(parentId, childId)` | Calls `FlexNode::addChild` |
| `removeChild(parentId, childId)` | Calls `FlexNode::removeChild` |
| `setText(id, text)` | Clears children and adds a `TextNode` |
| `setStyle(id, key, value)` | String key/value; subset of layout/color (see implementation) |
| `setRoot(id)` | Marks the root to return to C++ (see below) |

## RenderReactApp

```cpp
#include <AureliaReact.hpp>

FlexNode::Ptr root = AureliaUI::RenderReactApp(engine, jsSource, "bundle.js");
```

Workflow:

1. Installs `AureliaUI` globals.
2. Runs the script.
3. Returns the node registered with `AureliaUI.setRoot(handle)` (one-shot `takePendingRoot`).

The JS bundle should end by calling `AureliaUI.setRoot(...)` with the handle of the root flex node.

## modules/reactui (TypeScript)

Source lives under `modules/reactui/`. It uses `react` and `react-reconciler` to call `globalThis.AureliaUI`.

- Entry: `index.ts` exports `mount(element)`.
- Build (when Bun is available): `bun run bundle` → `bundle.js` (see `package.json`).

V8 does not include npm by default; ship a pre-bundled script or generate `bundle.js` in your pipeline.

## Dependencies (linking)

The JavaScript engine links **V8 monolith**, **Skia stack via GraphicComponent** (for `TextNode`), and Windows **`dbghelp.lib`**, **`winmm.lib`** for V8’s stack trace and timing helpers.

## Related

- [MikoUI](MikoUI.md) — Minimal `WinMain`-style helper.
- [Build and CI](BuildAndCI.md) — V8 prebuilt path and caching.
