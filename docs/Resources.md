# Resources (`res://`)

The `ResourceManager` maps string keys to embedded byte blobs, typically registered at startup from generated C++.

## Code

- `include/core/ResourceManager.hpp`
- `src/core/ResourceManager.cpp`

API:

- `registerResource(path, data, size)`
- `getResourceString("res://...")` — returns `std::string` payload

## Generation

`scripts/gen_resources.ts` reads an XML manifest of `<file>relative/path</file>` entries and emits:

- `BinaryResources.hpp` / `BinaryResources.cpp` (or per-app names such as `ReactUIResources`)

Each file becomes `registerResource("res://<relative/path>", ...)`.

The shared SVG bundle for icons is driven from `example/shared/resources/resrouces.xml` at the root CMake level.

## App pattern (ReactUI)

`example/ReactUI/resource.xml` lists app assets such as `bundle.js`. CMake runs `gen_resources.ts` with an init function name, compiles the generated `.cpp`, and `main.cpp` calls `InitReactUIResources()` before loading `res://bundle.js` into the JavaScript engine.

## Icon / SVG usage

`IconNode::setIcon("res://settings.svg")` resolves through `ResourceManager` when the SVG string is registered.

See [GUI Components](GuiComponents.md) for `IconNode`.
