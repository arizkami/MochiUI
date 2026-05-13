# SphereUI Documentation

Welcome to the SphereUI documentation. SphereUI is a modern, high-performance C++ UI framework designed for desktop applications.

## Key Features

- **Skia-powered Rendering**: High-performance, cross-platform graphics.
- **Flexbox Layout**: Powered by Yoga for flexible and responsive designs.
- **Modern Themes**: Built-in dark and light palettes.
- **Component-based Architecture**: Easy-to-use nodes for building complex UIs.
- **Event System**: Robust event handling for user interactions.
- **Optional V8 + React bridge**: JavaScript `eval`, native `SphereUI` bindings, and `Sphere.React` ([JavaScriptAndReact.md](JavaScriptAndReact.md)).
- **Embedded resources**: `res://` URIs and generated binary bundles ([Resources.md](Resources.md)).

## Project Structure

- `include/`: Public API headers (`SPHX*.hpp`, `core/`, `gui/`, …).
- `src/`: Implementation (`gui/`, `core/`, `platform/`, `javascript/`, `react/`, …).
- `example/`: Sample applications (`Mixer`, `ReactUI`, …).
- `framework/MikoUI/`: Minimal “MikoUI” helper headers ([MikoUI.md](MikoUI.md)).
- `modules/reactui/`: TypeScript React reconciler host with `src/` internals and `examples/` demo entrypoints.
- `external/`: Third-party and prebuilt drops (Skia, Yoga, V8 under `external/prebuilt/v8`, …).
- `docs/`: Markdown documentation; `docs/mdx/` holds **MDX** copies for site generators.
- `scripts/`: Resource and tooling helpers (`gen_resources.ts`, etc.).

## Getting Started

To get started with SphereUI, please refer to the [Getting Started](GettingStarted.md) guide. For builds and CI, see [Build and CI](BuildAndCI.md).
