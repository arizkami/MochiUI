#pragma once
#include <SPHXFoundation.hpp>
#include <cmath>

// SPHXPainter — fluent, high-level 2-D drawing API wrapping SkCanvas.
//
// Construct on the canvas passed to FlexNode::draw(), then chain calls:
//
//   SPHXPainter p(canvas);
//   p.shadow(frame, 10.f, SPHXColor::black().withAlpha(80))
//    .roundRect(frame, 10.f, P::Surface)
//    .roundRectBorder(frame, 10.f, P::Border);
//
// Gradient helpers use the modern SkShaders:: / SkGradient API (Skia 2025+).
// Shadow helpers use SkMaskFilter::MakeBlur.

namespace SphereUI {

class SPHXPainter {
public:
    explicit SPHXPainter(SkCanvas* canvas) : _c(canvas) {}

    // ── Raw access ────────────────────────────────────────────────────────────
    SkCanvas* raw() const { return _c; }

    // ── Canvas state ──────────────────────────────────────────────────────────
    SPHXPainter& save()    { _c->save();    return *this; }
    SPHXPainter& restore() { _c->restore(); return *this; }

    SPHXPainter& translate(float dx, float dy) {
        _c->translate(dx, dy); return *this;
    }
    SPHXPainter& rotate(float degrees, float cx = 0.f, float cy = 0.f) {
        _c->rotate(degrees, cx, cy); return *this;
    }
    SPHXPainter& scale(float sx, float sy) {
        _c->scale(sx, sy); return *this;
    }

    // ── Clipping ──────────────────────────────────────────────────────────────
    SPHXPainter& clip(const SkRect& r, bool aa = true) {
        _c->clipRect(r, aa); return *this;
    }
    SPHXPainter& clipRounded(const SkRect& r, float radius, bool aa = true) {
        _c->clipRRect(SkRRect::MakeRectXY(r, radius, radius), SkClipOp::kIntersect, aa);
        return *this;
    }
    SPHXPainter& clipRRect(const SkRRect& rr, bool aa = true) {
        _c->clipRRect(rr, SkClipOp::kIntersect, aa); return *this;
    }
    // Per-corner clip: top-left, top-right, bottom-right, bottom-left
    SPHXPainter& clipRoundedCorners(const SkRect& r,
                                   float tl, float tr, float br, float bl, bool aa = true) {
        const SkVector radii[4] = {{tl,tl},{tr,tr},{br,br},{bl,bl}};
        SkRRect rr; rr.setRectRadii(r, radii);
        _c->clipRRect(rr, SkClipOp::kIntersect, aa); return *this;
    }
    SPHXPainter& clipCircle(float cx, float cy, float r, bool aa = true) {
        _c->clipRRect(
            SkRRect::MakeOval(SkRect::MakeXYWH(cx - r, cy - r, r * 2.f, r * 2.f)),
            SkClipOp::kIntersect, aa);
        return *this;
    }

    // ── Filled shapes ─────────────────────────────────────────────────────────
    SPHXPainter& rect(const SkRect& r, SPHXColor c) {
        _c->drawRect(r, fill(c)); return *this;
    }
    SPHXPainter& roundRect(const SkRect& r, float radius, SPHXColor c) {
        _c->drawRoundRect(r, radius, radius, fill(c)); return *this;
    }
    SPHXPainter& roundRect(const SkRect& r, float rx, float ry, SPHXColor c) {
        _c->drawRoundRect(r, rx, ry, fill(c)); return *this;
    }
    SPHXPainter& rrect(const SkRRect& rr, SPHXColor c) {
        _c->drawRRect(rr, fill(c)); return *this;
    }
    // Per-corner radii: top-left, top-right, bottom-right, bottom-left
    SPHXPainter& roundRectCorners(const SkRect& r,
                                 float tl, float tr, float br, float bl, SPHXColor c) {
        const SkVector radii[4] = {{tl,tl},{tr,tr},{br,br},{bl,bl}};
        SkRRect rr; rr.setRectRadii(r, radii);
        _c->drawRRect(rr, fill(c)); return *this;
    }
    SPHXPainter& circle(float cx, float cy, float r, SPHXColor c) {
        _c->drawCircle(cx, cy, r, fill(c)); return *this;
    }
    SPHXPainter& path(const SkPath& path, SPHXColor c) {
        _c->drawPath(path, fill(c)); return *this;
    }

    // ── Stroked shapes ────────────────────────────────────────────────────────
    SPHXPainter& rectBorder(const SkRect& r, SPHXColor c, float w = 1.f) {
        _c->drawRect(r, stroke(c, w)); return *this;
    }
    SPHXPainter& roundRectBorder(const SkRect& r, float radius, SPHXColor c, float w = 1.f) {
        _c->drawRoundRect(r, radius, radius, stroke(c, w)); return *this;
    }
    SPHXPainter& rrectBorder(const SkRRect& rr, SPHXColor c, float w = 1.f) {
        _c->drawRRect(rr, stroke(c, w)); return *this;
    }
    SPHXPainter& circleBorder(float cx, float cy, float r, SPHXColor c, float w = 1.f) {
        _c->drawCircle(cx, cy, r, stroke(c, w)); return *this;
    }
    SPHXPainter& pathStroke(const SkPath& p, SPHXColor c, float w = 1.f,
                           SkPaint::Cap cap = SkPaint::kRound_Cap) {
        auto pk = stroke(c, w); pk.setStrokeCap(cap);
        _c->drawPath(p, pk); return *this;
    }

    // ── Lines ─────────────────────────────────────────────────────────────────
    SPHXPainter& line(float x1, float y1, float x2, float y2, SPHXColor c, float w = 1.f,
                     SkPaint::Cap cap = SkPaint::kRound_Cap) {
        auto p = stroke(c, w); p.setStrokeCap(cap);
        _c->drawLine(x1, y1, x2, y2, p); return *this;
    }
    SPHXPainter& line(SkPoint a, SkPoint b, SPHXColor c, float w = 1.f) {
        _c->drawLine(a, b, stroke(c, w)); return *this;
    }

    // ── Arcs (degrees, 0 = 3 o'clock, clockwise) ─────────────────────────────
    SPHXPainter& arc(const SkRect& oval, float startAngle, float sweepAngle,
                    SPHXColor c, float w = 2.f,
                    SkPaint::Cap cap = SkPaint::kRound_Cap) {
        auto p = stroke(c, w); p.setStrokeCap(cap);
        _c->drawArc(oval, startAngle, sweepAngle, false, p); return *this;
    }
    // Filled wedge connected to centre
    SPHXPainter& pie(const SkRect& oval, float startAngle, float sweepAngle, SPHXColor c) {
        _c->drawArc(oval, startAngle, sweepAngle, true, fill(c)); return *this;
    }

    // ── Linear gradient ───────────────────────────────────────────────────────
    // angleDeg: 0 = left→right, 90 = top→bottom (default)
    SPHXPainter& linearGradient(const SkRect& r, SPHXColor from, SPHXColor to,
                               float angleDeg = 90.f) {
        _c->drawRect(r, makeLinear(r, from, to, angleDeg)); return *this;
    }
    SPHXPainter& roundRectGradient(const SkRect& r, float radius,
                                  SPHXColor from, SPHXColor to, float angleDeg = 90.f) {
        _c->drawRoundRect(r, radius, radius, makeLinear(r, from, to, angleDeg));
        return *this;
    }
    SPHXPainter& circleGradient(float cx, float cy, float r,
                               SPHXColor from, SPHXColor to, float angleDeg = 90.f) {
        SkRect bounds = SkRect::MakeXYWH(cx - r, cy - r, r * 2.f, r * 2.f);
        _c->drawCircle(cx, cy, r, makeLinear(bounds, from, to, angleDeg));
        return *this;
    }

    // ── Radial gradient ───────────────────────────────────────────────────────
    // Draws a filled circle shading from centre colour to edge colour.
    SPHXPainter& radialGradient(float cx, float cy, float r,
                               SPHXColor center, SPHXColor edge) {
        SkColor4f cols[2] = { toF(center), toF(edge) };
        SkGradient::Colors gc(SkSpan<const SkColor4f>(cols, 2), SkTileMode::kClamp);
        SkPaint p; p.setAntiAlias(true);
        p.setShader(SkShaders::RadialGradient(SkPoint{cx, cy}, r,
                        SkGradient(gc, SkGradient::Interpolation{})));
        _c->drawCircle(cx, cy, r, p); return *this;
    }
    // Fills a rect; radius = shortest half-extent of the rect.
    SPHXPainter& radialGradientRect(const SkRect& r, SPHXColor center, SPHXColor edge) {
        float cx = r.centerX(), cy = r.centerY();
        float rad = std::min(r.width(), r.height()) * 0.5f;
        SkColor4f cols[2] = { toF(center), toF(edge) };
        SkGradient::Colors gc(SkSpan<const SkColor4f>(cols, 2), SkTileMode::kClamp);
        SkPaint p; p.setAntiAlias(true);
        p.setShader(SkShaders::RadialGradient(SkPoint{cx, cy}, rad,
                        SkGradient(gc, SkGradient::Interpolation{})));
        _c->drawRect(r, p); return *this;
    }

    // ── Sweep (conic) gradient ────────────────────────────────────────────────
    SPHXPainter& sweepGradient(float cx, float cy, float r,
                              SPHXColor from, SPHXColor to,
                              float startAngle = 0.f, float endAngle = 360.f) {
        SkColor4f cols[2] = { toF(from), toF(to) };
        SkGradient::Colors gc(SkSpan<const SkColor4f>(cols, 2), SkTileMode::kClamp);
        SkPaint p; p.setAntiAlias(true);
        p.setShader(SkShaders::SweepGradient(SkPoint{cx, cy}, startAngle, endAngle,
                        SkGradient(gc, SkGradient::Interpolation{})));
        _c->drawCircle(cx, cy, r, p); return *this;
    }

    // ── Shadow (Gaussian blur) ────────────────────────────────────────────────
    // Draw a blurred rounded rect behind content; sigma controls softness.
    SPHXPainter& shadow(const SkRect& r, float cornerRadius, SPHXColor c,
                       float sigma = 4.f, float dx = 0.f, float dy = 3.f) {
        SkPaint p; p.setAntiAlias(true);
        p.setColor(c.toSkColor());
        p.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma));
        _c->drawRoundRect(r.makeOffset(dx, dy), cornerRadius, cornerRadius, p);
        return *this;
    }
    SPHXPainter& shadowCircle(float cx, float cy, float r, SPHXColor c,
                             float sigma = 4.f, float dx = 0.f, float dy = 3.f) {
        SkPaint p; p.setAntiAlias(true);
        p.setColor(c.toSkColor());
        p.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma));
        _c->drawCircle(cx + dx, cy + dy, r, p); return *this;
    }

    // ── Escape hatch ──────────────────────────────────────────────────────────
    SPHXPainter& withPaint(const SkPaint& paint,
                          void (*fn)(SkCanvas*, const SkPaint&)) {
        fn(_c, paint); return *this;
    }

private:
    SkCanvas* _c;

    static SkPaint fill(SPHXColor c) {
        SkPaint p; p.setAntiAlias(true);
        p.setColor(c.toSkColor()); return p;
    }
    static SkPaint stroke(SPHXColor c, float w) {
        SkPaint p; p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(w);
        p.setColor(c.toSkColor()); return p;
    }
    static SkColor4f toF(SPHXColor c) {
        return { c.rf(), c.gf(), c.bf(), c.af() };
    }
    static SkPaint makeLinear(const SkRect& r, SPHXColor from, SPHXColor to, float angleDeg) {
        float rad = angleDeg * SK_FloatPI / 180.f;
        float cx = r.centerX(), cy = r.centerY();
        float hw = r.width()  * 0.5f * std::cos(rad);
        float hh = r.height() * 0.5f * std::sin(rad);
        SkPoint pts[2] = { SkPoint{cx - hw, cy - hh}, SkPoint{cx + hw, cy + hh} };
        SkColor4f cols[2] = { toF(from), toF(to) };
        SkGradient::Colors gc(SkSpan<const SkColor4f>(cols, 2), SkTileMode::kClamp);
        SkPaint p; p.setAntiAlias(true);
        p.setShader(SkShaders::LinearGradient(pts, SkGradient(gc, SkGradient::Interpolation{})));
        return p;
    }
};

} // namespace SphereUI
