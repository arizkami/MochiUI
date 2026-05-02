#pragma once
#include <AUKFoundation.hpp>

// AUKSvg — SVG document loading and rendering via Skia's SVGDOM.
// Supports SVG 1.1: paths, gradients, filters, transforms, clip-paths, and
// basic text. Loads from file, memory, string literals, or res:// resources.
//
//   auto logo = AUKSvg::fromFile("assets/logo.svg");
//   auto icon = AUKSvg::fromResource("res://play.svg");
//
//   void MyNode::draw(SkCanvas* canvas) override {
//       logo.render(canvas, frame);                  // scale to node rect
//       icon.render(canvas, 8.f, 8.f, 24.f, 24.f);  // explicit size
//   }
//
// Note: SkSVGDOM and its dependencies are hidden behind pimpl; you do not
// need to include any Skia SVG module headers to use this class.

namespace AureliaUI {

class AUKSvg {
public:
    AUKSvg() = default;

    // ── Factories ─────────────────────────────────────────────────────────────
    static AUKSvg fromFile(const std::string& path);
    static AUKSvg fromMemory(const void* data, size_t size);
    static AUKSvg fromString(const std::string& svgText);
    static AUKSvg fromResource(const std::string& resUri);   // "res://name"

    // ── State ─────────────────────────────────────────────────────────────────
    bool   isValid() const { return _impl != nullptr; }
    // Returns the SVG's declared width/height (0 if expressed in percent)
    SkSize intrinsicSize() const;
    float  width()  const { return intrinsicSize().width();  }
    float  height() const { return intrinsicSize().height(); }

    // ── Rendering ─────────────────────────────────────────────────────────────
    // Render at intrinsic size with top-left at (x, y)
    void render(SkCanvas* canvas, float x = 0.f, float y = 0.f) const;

    // Scale to fill dst rectangle
    void render(SkCanvas* canvas, const SkRect& dst) const;

    // Render at an explicit position and size
    void render(SkCanvas* canvas, float x, float y, float w, float h) const;

    // Render at explicit size with opacity (0.0–1.0)
    void render(SkCanvas* canvas, const SkRect& dst, float opacity) const;

    // Render a single named element by id (e.g. "#icon-path")
    void renderNode(SkCanvas* canvas, const SkRect& dst, const char* nodeId) const;

    struct Impl;
private:
    std::shared_ptr<Impl> _impl;
    explicit AUKSvg(std::shared_ptr<Impl> impl) : _impl(std::move(impl)) {}
};

} // namespace AureliaUI
