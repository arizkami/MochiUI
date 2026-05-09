# `@spherekit/react` (React UI runtime)

React wrapper for SphereEngine / SphereKit. This package provides a small React reconciler and a React-Native-like component surface that targets the engine’s `globalThis.SphereUI` bridge (created by `SphereKit.JavaScriptEngine` on the C++ side).

## Exports

- **Named exports**: `mount`, `StyleSheet`, and components like `View`, `Text`, `Button`, `ScrollView`, `TextInput`, `Switch`, `Slider`, `Image`, `FlatList`, etc.
- **Default export**: a `Sphere` object containing the same helpers/components (see `src/index.ts`).

## Requirements

- **Bun** (used for bundling/building)
- **TypeScript** (peer dependency)

## Install

From this repo, in `modules/reactui/`:

```bash
bun install
```

## Build (library output)

Generates ESM output + `.d.ts` into `dist/`:

```bash
bun run build
```

## Bundle (engine consumption)

For embedding into the engine, you typically want a single-file JS bundle.

- **ESM bundle** (for Bun-targeted workflows):

```bash
bun run bundle
```

- **Demo bundle** (IIFE, written into `example/ReactUI/bundle.js` at repo root):

```bash
bun run bundle:demo
```

## How it’s used (high level)

1. **TypeScript/React** code is bundled into a single JS file.
2. C++ initializes `SphereKit.JavaScriptEngine` and installs `globalThis.SphereUI`.
3. The engine `eval`s the bundle and your app calls `mount(...)` to create/set the UI root via the bridge.

If you’re looking for a working end-to-end example, start with:

- `modules/reactui/examples/` (React entrypoints)
- `example/ReactUI/` (bundle output target from `bundle:demo`)
