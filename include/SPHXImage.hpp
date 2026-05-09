#pragma once
#include <SPHXFoundation.hpp>

// SPHXImage — image loading and drawing.
// Decodes PNG, JPEG, WebP, BMP, GIF, AVIF and any other format supported by
// the bundled Skia codecs. Supports file paths, raw memory buffers, and the
// res:// embedded-resource protocol (populated by InitBinaryResources()).
//
//   auto banner = SPHXImage::fromFile("assets/banner.png");
//   auto icon   = SPHXImage::fromResource("res://logo.png");
//
//   void MyNode::draw(SkCanvas* canvas) override {
//       banner.draw(canvas, frame);                     // stretch to node rect
//       icon.draw(canvas, frame.left(), frame.top());   // natural size at pos
//   }

namespace SphereUI {

class SPHXImage {
public:
    SPHXImage() = default;

    // ── Factories ─────────────────────────────────────────────────────────────
    static SPHXImage fromFile(const std::string& path);
    static SPHXImage fromMemory(const void* data, size_t size);
    static SPHXImage fromResource(const std::string& resUri);  // "res://name"

    // ── State ─────────────────────────────────────────────────────────────────
    bool    isValid()     const { return _image != nullptr; }
    int     width()       const { return _image ? _image->width()      : 0; }
    int     height()      const { return _image ? _image->height()     : 0; }
    SkISize dimensions()  const { return _image ? _image->dimensions() : SkISize::MakeEmpty(); }
    float   aspectRatio() const {
        return (_image && _image->height() > 0)
            ? static_cast<float>(_image->width()) / static_cast<float>(_image->height())
            : 1.f;
    }

    // ── Drawing ───────────────────────────────────────────────────────────────
    // Natural pixel size, top-left at (x, y)
    void draw(SkCanvas* canvas, float x, float y,
              SkSamplingOptions sampling = SkSamplingOptions(SkFilterMode::kLinear)) const;

    // Stretch to fill dst
    void draw(SkCanvas* canvas, const SkRect& dst,
              SkSamplingOptions sampling = SkSamplingOptions(SkFilterMode::kLinear)) const;

    // Sub-region src → dst (atlas, nine-patch)
    void draw(SkCanvas* canvas, const SkRect& src, const SkRect& dst,
              SkSamplingOptions sampling = SkSamplingOptions(SkFilterMode::kLinear)) const;

    // Stretch to fill dst at given opacity (0.0–1.0)
    void draw(SkCanvas* canvas, const SkRect& dst, float opacity,
              SkSamplingOptions sampling = SkSamplingOptions(SkFilterMode::kLinear)) const;

    // Scale to fit inside dst preserving aspect ratio (letterbox / contain)
    void drawFit(SkCanvas* canvas, const SkRect& dst,
                 SkSamplingOptions sampling = SkSamplingOptions(SkFilterMode::kLinear)) const;

    // Scale to fill dst preserving aspect ratio, clipping excess (cover)
    void drawFill(SkCanvas* canvas, const SkRect& dst,
                  SkSamplingOptions sampling = SkSamplingOptions(SkFilterMode::kLinear)) const;

    // ── Raw Skia access ───────────────────────────────────────────────────────
    sk_sp<SkImage> skImage() const { return _image; }

private:
    sk_sp<SkImage> _image;
    explicit SPHXImage(sk_sp<SkImage> img) : _image(std::move(img)) {}

    static sk_sp<SkImage> decodeFromData(sk_sp<SkData> data);
};

} // namespace SphereUI
