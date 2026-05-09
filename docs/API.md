# SphereUI API Documentation

This is the central index for the SphereUI documentation.

## Documentation Modules

- [**Introduction**](Introduction.md): Overview and features of SphereUI.
- [**Getting Started**](GettingStarted.md): A step-by-step guide to building your first app.
- [**Build and CI**](BuildAndCI.md): Local `build.ps1`, Skia/V8 prebuilts, GitHub Actions.
- [**Core API**](CoreAPI.md): Reference for main classes like `Application`, `Window`, and `FlexNode`.
- [**GUI Components**](GuiComponents.md): Catalog of built-in UI elements.
- [**Theming**](Theming.md): Guide to the theme system and customization.
- [**Layout System**](Layout.md): Deep dive into the Flexbox-based layout system.
- [**Audio Streaming**](Audio.md): High-level audio playback and streaming.
- [**Resources**](Resources.md): `res://` URIs, `ResourceManager`, `gen_resources.py`.
- [**JavaScript and React**](JavaScriptAndReact.md): `JavaScriptEngine`, `Sphere.React`, `SphereKit.Vue`, `globalThis.SphereUI`.
- [**MikoUI**](MikoUI.md): Minimal framework helper (`framework/MikoUI`).

### MDX (site-friendly)

For static-site generators that consume **MDX**, see [`docs/mdx/`](mdx/index.mdx) (index and topic pages).

## Namespaces

- `SphereUI`: The main namespace for all framework classes.
- `SphereUI::Theme`: Namespace for active theme tokens.

## Common Types

- `FlexNode::Ptr`: Shared pointer to a UI node (`std::shared_ptr<FlexNode>`).
- `SPHXColor`: High-level color representation with Hex, RGB, and HSL support.
