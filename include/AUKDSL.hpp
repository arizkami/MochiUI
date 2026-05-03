#pragma once
#include <AUKGraphicComponents.hpp>
#include <string>
#include <memory>

// ── AUKDSL ────────────────────────────────────────────────────────────────────
//
// Declarative UI for AureliaKit. Parses a CSS-like YAML document and builds a
// FlexNode tree that can be handed straight to `Win32Window::setRoot()`.
//
//   navbar:
//     type: div
//     props:
//       style: { display: flex, alignItems: center, padding: "0 16px" }
//       text:  "Optional inline content"
//     children:
//       - type: span
//         props:
//           style: { color: "#9ca3af" }
//           text:  "Hello"
//
// `props.style` may be a YAML map (camelCase keys) or a single string of CSS
// declarations parsed with libcss, e.g.:
//   style: "display: flex; align-items: center; height: 64px; color: #fff;"
//
// Supported style keys (CSS subset):
//   width, height,
//   display (flex/block),
//   flexDirection (row/column/row-reverse/column-reverse),
//   alignItems (center/flex-start/flex-end/stretch),
//   justifyContent (center/flex-start/flex-end/space-between/space-around/space-evenly),
//   gap, padding, margin,
//   backgroundColor, color,
//   fontFamily, fontSize, fontWeight, letterSpacing,
//   border, borderTop, borderRight, borderBottom, borderLeft,
//   borderRadius, borderColor, borderWidth,
//   cursor (pointer/text/default), overflow (hidden/visible),
//   flex, flexGrow, flexShrink, flexBasis, flexWrap.
//
// Supported types: div (block-ish container), span (inline-ish container).
// Both map onto `DSLNode`; the difference is purely cosmetic since layout is
// driven entirely by the `style` map.
//
// Children placement is permissive — `children:` may sit either alongside
// `props:` or inside it; both forms work.

namespace AureliaUI {

// FlexNode-derived element produced by the DSL parser. Adds inline text
// rendering and per-edge borders so a single node can model a CSS box.
class DSLNode : public FlexNode {
public:
    using Ptr = std::shared_ptr<DSLNode>;

    // ── Inline text ───────────────────────────────────────────────────────────
    std::string text;
    std::string fontFamily = FontManager::DEFAULT_FONT;
    AUKColor    textColor  = Theme::TextPrimary;
    float       fontSize   = 14.0f;
    bool        fontBold   = false;
    TextAlign   textAlign  = TextAlign::Left;
    // Extra horizontal advance between grapheme clusters (CSS `letter-spacing`),
    // in CSS px. Measured/drawn only for simple left-to-right runs.
    float       letterSpacing = 0.0f;

    // ── Border (per edge) ────────────────────────────────────────────────────
    float    borderTopWidth    = 0.0f;
    float    borderRightWidth  = 0.0f;
    float    borderBottomWidth = 0.0f;
    float    borderLeftWidth   = 0.0f;
    AUKColor borderTopColor    = AUKColor::transparent();
    AUKColor borderRightColor  = AUKColor::transparent();
    AUKColor borderBottomColor = AUKColor::transparent();
    AUKColor borderLeftColor   = AUKColor::transparent();

    DSLNode();

    // True once the parser commits to "leaf with text" (no children).
    // Yoga only honours measure callbacks on leaf nodes, so this flips the
    // behaviour of `measure()` on/off depending on tree shape.
    void setLeafTextNode(bool leaf);

    Size measure(Size available) override;
    void drawSelf(SkCanvas* canvas) override;
    void draw(SkCanvas* canvas) override;
};

namespace DSL {

// Parse a YAML string into a FlexNode tree.
//
// The document root may be either the node spec itself, or a single-key map
// whose value is the node spec (e.g. `navbar: { type: div, ... }`). Returns
// nullptr on parse failure.
FlexNode::Ptr loadFromString(const std::string& yaml);

// Read & parse a YAML file from disk. Returns nullptr if the file cannot be
// opened or parsing fails.
FlexNode::Ptr loadFromFile(const std::string& path);

} // namespace DSL
} // namespace AureliaUI
