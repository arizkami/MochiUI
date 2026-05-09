# JavaScript engine, Sphere.React, and SphereKit.Vue

Windows builds include a V8-based embed and bridge DLLs for driving `FlexNode` trees from JavaScript, including both the React reconciler host and the Vue custom renderer host.

## Deliverables

| CMake target | Output name | Role |
|--------------|-------------|------|
| `SphereKit_JavaScriptEngine` | `SphereKit.JavaScriptEngine.dll` | V8 isolate, `eval()`, native `SphereUI` bindings |
| `Sphere_React` | `Sphere.React.dll` | Thin API: `RenderReactApp(...)` |
| `Sphere_Vue` | `SphereKit.Vue.dll` | Thin API: `RenderVueApp(...)` |

Headers:

- `include/SPHXJavaScriptEngine.hpp` — `JavaScriptEngine`
- `include/SphereReact.hpp` — `RenderReactApp`
- `include/SphereVUE.hpp` — `RenderVueApp`

## JavaScriptEngine

```cpp
SphereUI::JavaScriptEngine engine;
engine.init();
engine.installSphereUIGlobal();  // defines globalThis.SphereUI
bool ok = engine.eval(script, "my.js");
// on failure: engine.lastError()
```

Shut down when done: `engine.shutdown()`.

## Native binding: `globalThis.SphereUI`

Numeric handles identify `FlexNode` instances in a per-isolate registry.

| Method | Description |
|--------|-------------|
| `createNode(type: string) → number` | Creates a node. Types: `column`, `row`, `div` (column flex), or other (generic `FlexNode::Create()`) |
| `appendChild(parentId, childId)` | Calls `FlexNode::addChild` |
| `removeChild(parentId, childId)` | Calls `FlexNode::removeChild` |
| `setText(id, text)` | Clears children and adds a `TextNode` |
| `setStyle(id, key, value)` | String key/value; subset of layout/color (see implementation) |
| `setRoot(id)` | Marks the root to return to C++ (see below) |

## RenderReactApp / RenderVueApp

```cpp
#include <SphereReact.hpp>
#include <SphereVUE.hpp>

FlexNode::Ptr root = SphereUI::RenderReactApp(engine, jsSource, "bundle.js");
FlexNode::Ptr vueRoot = SphereUI::RenderVueApp(engine, jsSource, "bundle.js");
```

Workflow:

1. Installs `SphereUI` globals.
2. Runs the script.
3. Returns the node registered with `SphereUI.setRoot(handle)` (one-shot `takePendingRoot`).

The JS bundle should end by calling `SphereUI.setRoot(...)` with the handle of the root flex node.

## TypeScript hosts

React source lives under `modules/reactui/`. It uses `react` and `react-reconciler` to call `globalThis.SphereUI`.

- Public entry: `src/index.ts` exports `mount(element)`.
- Internal layout: `src/core/`, `src/payload/`, and `src/wrapper/`.
- Demo entry: `examples/App.tsx`.
- Build (when Bun is available): `bun run bundle` → `bundle.js` (see `package.json`).

Vue source lives under `modules/vueui/`. It uses Vue 3's custom renderer API to call the same `globalThis.SphereUI` methods.

- Public entry: `src/index.ts`.
- Internal layout: `src/core/`, `src/payload/`, and `src/wrapper/`.
- Demo entry: `example/App.vue`.
- Build demo (when Bun is available): `bun run bundle:demo` → `example/VueUI/bundle.js`.

V8 does not include npm by default; ship a pre-bundled script or generate `bundle.js` in your pipeline.

## Dependencies (linking)

The JavaScript engine links **V8 monolith**, **Skia stack via GraphicComponent** (for `TextNode`), and Windows **`dbghelp.lib`**, **`winmm.lib`** for V8’s stack trace and timing helpers.

## Related

- [MikoUI](MikoUI.md) — Minimal `WinMain`-style helper.
- [Build and CI](BuildAndCI.md) — V8 prebuilt path and caching.
