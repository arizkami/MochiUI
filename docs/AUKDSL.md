# AUKDSL — Declarative UI (YAML)

AUKDSL loads a YAML document and produces a tree of `DSLNode` objects (subclasses of `FlexNode`) that you pass to `Win32Window::setRoot()`.

## Headers and API

- `include/AUKDSL.hpp` — `DSLNode`, namespace `AureliaUI::DSL`
- `DSL::loadFromString(const std::string& yaml)` → `FlexNode::Ptr` or `nullptr`
- `DSL::loadFromFile(const std::string& path)` → same

The document root may be either a node map directly, or a single-key wrapper (e.g. `navbar:`) whose value is the node spec.

## Node shape

Each node has:

- `type`: `div` or `span` (same underlying `DSLNode`; behavior is style-driven).
- `props`: optional map with:
  - `style`: YAML map of style keys **or** a single CSS string (parsed via libcss).
  - `text`: inline text on this node (optional; leaf text nodes get a Yoga measure callback).
- `children`: sequence of child nodes (may also appear next to `props` per parser rules).

## Style keys (YAML map, camelCase)

Supported keys include:

| Category | Keys |
|----------|------|
| Size | `width`, `height`, `minWidth`, `minHeight` |
| Flex | `display`, `flexDirection`, `alignItems`, `justifyContent`, `flex`, `flexGrow`, `flexShrink`, `flexBasis`, `flexWrap`, `gap` |
| Box | `padding`, `paddingTop`, …, `margin`, … |
| Appearance | `backgroundColor`, `color`, `borderRadius`, `overflow` |
| Border | `border`, `borderTop`, …, `borderWidth`, `borderColor` |
| Typography | `fontFamily`, `fontSize`, `fontWeight`, `letterSpacing`, `textAlign` |
| Cursor | `cursor` |

CSS string form is also supported on `style`, e.g.:

```yaml
style: "display: flex; align-items: center; height: 64px;"
```

## Example

See `example/DSLDemo/ui/` for full samples. Minimal fragment:

```yaml
screen:
  type: div
  props:
    style:
      display: "flex"
      flexDirection: "column"
      width: "100%"
      height: "100%"
      backgroundColor: "#0b0f14"
  children:
    - type: span
      props:
        style: { color: "#ffffff", fontSize: "14px" }
        text: "Hello"
```

## Related

- [Layout](Layout.md) — Yoga / `FlexNode` layout.
- [Resources](Resources.md) — Loading UI text from `res://` bundles (DSLDemo pattern).
