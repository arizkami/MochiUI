# Layout System

AureliaUI uses the **Yoga** layout engine, which implements the **CSS Flexbox** specification. This allows for highly flexible and responsive user interfaces.

## FlexBox Basics

Every `FlexNode` has a `LayoutStyle` that defines how it and its children are laid out.

### Flex Direction

Determines the primary axis for children.

- `Row`: Children are laid out horizontally.
- `Column`: Children are laid out vertically (default).

```cpp
auto root = FlexNode::Row(); // Children will be side-by-side
```

### Justify Content

Defines how children are distributed along the primary axis.

- `YGJustifyFlexStart`: Aligned to the start.
- `YGJustifyCenter`: Centered.
- `YGJustifyFlexEnd`: Aligned to the end.
- `YGJustifySpaceBetween`: Distributed with equal space between them.

### Align Items

Defines how children are aligned along the cross axis.

- `YGAlignFlexStart`: Aligned to the start.
- `YGAlignCenter`: Centered.
- `YGAlignFlexEnd`: Aligned to the end.
- `YGAlignStretch`: Stretched to fill the container.

## Sizing Modes

Nodes can have different sizing behaviors:

- **Fixed**: Explicit width/height in pixels.
- **Flex**: Expands to fill available space (controlled by `setFlex()`).
- **Hug** (Auto): Shrinks to fit its content.

```cpp
auto box = FlexNode::Create();
box->style.setWidth(100);    // Fixed
box->style.setFlex(1.0f);    // Fills available space
box->style.setWidthAuto();   // Hugs content
```

## Spacing

- **Padding**: Internal space within a node.
- **Margin**: External space around a node.
- **Gap**: Space between children.

```cpp
node->style.setPadding(10); // 10px on all sides
node->style.setGap(8);      // 8px between children
```
