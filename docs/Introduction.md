# AureliaUI Documentation

Welcome to the AureliaUI documentation. AureliaUI is a modern, high-performance C++ UI framework designed for desktop applications.

## Key Features

- **Skia-powered Rendering**: High-performance, cross-platform graphics.
- **Flexbox Layout**: Powered by Yoga for flexible and responsive designs.
- **Modern Themes**: Support for multiple themes including Dark, Light, Material Design 3, and Windows-style themes.
- **Component-based Architecture**: Easy-to-use nodes for building complex UIs.
- **Event System**: Robust event handling for user interactions.
- **AUKDSL**: Declarative YAML/CSS UI definitions ([AUKDSL.md](AUKDSL.md)).
- **Optional V8 + React bridge**: JavaScript `eval`, native `AureliaUI` bindings, and `Aurelia.React` ([JavaScriptAndReact.md](JavaScriptAndReact.md)).
- **Embedded resources**: `res://` URIs and generated binary bundles ([Resources.md](Resources.md)).

## Project Structure

- `include/`: Public API headers (`AUK*.hpp`, `core/`, `gui/`, …).
- `src/`: Implementation (`gui/`, `core/`, `platform/`, `dsl/`, `javascript/`, `react/`, …).
- `example/`: Sample applications (`DSLDemo`, `Mixer`, …).
- `framework/MikoUI/`: Minimal “MikoUI” helper headers ([MikoUI.md](MikoUI.md)).
- `modules/reactui/`: TypeScript React reconciler host (calls native `AureliaUI`).
- `external/`: Third-party and prebuilt drops (Skia, Yoga, V8 under `external/prebuilt/v8`, …).
- `docs/`: Markdown documentation; `docs/mdx/` holds **MDX** copies for site generators.
- `scripts/`: Theme and resource codegen (`gen_theme.py`, `gen_resources.py`).

## Getting Started

To get started with AureliaUI, please refer to the [Getting Started](GettingStarted.md) guide. For builds and CI, see [Build and CI](BuildAndCI.md).
