#pragma once
#include <AUKFoundation.hpp>
#include <cmath>

// AUKPainter — fluent, high-level 2-D drawing API wrapping SkCanvas.
//
// Construct on the canvas passed to FlexNode::draw(), then chain calls:
//
//   AUKPainter p(canvas);
//   p.shadow(frame, 10.f, AUKColor::black().withAlpha(80))
//    .roundRect(frame, 10.f, P::Surface)
//    .roundRectBorder(frame, 10.f, P::Border);
//
// Gradient helpers use the modern SkShaders:: / SkGradient API (Skia 2025+).
// Shadow helpers use SkMaskFilter::MakeBlur.

namespace AureliaUI {

class AUKPainter {
public:
    explicit AUKPainter(SkCanvas* canvas) : _c(canvas) {}

    // ── Raw access ────────────────────────────────────────────────────────────
    SkCanvas* raw() const { return _c; }

    // ── Canvas state ──────────────────────────────────────────────────────────
    AUKPainter& save()    { _c->save();    return *this; }
    AUKPainter& restore() { _c->restore(); return *this; }

    AUKPainter& translate(float dx, float dy) {
        _c->translate(dx, dy); return *this;
    }
    AUKPainter& rotate(float degrees, float cx = 0.f, float cy = 0.f) {
        _c->rotate(degrees, cx, cy); return *this;
    }
    AUKPainter& scale(float sx, float sy) {
        _c->scale(sx, sy); return *this;
    }

    // ── Clipping ──────────────────────────────────────────────────────────────
    AUKPainter& clip(const SkRect& r, bool aa = true) {
        _c->clipRect(r, aa); return *this;
    }
    AUKPainter& clipRounded(const SkRect& r, float radius, bool aa = true) {
        _c->clipRRect(SkRRect::MakeRectXY(r, radius, radius), SkClipOp::kIntersect, aa);
        return *this;
    }
    AUKPainter& clipRRect(const SkRRect& rr, bool aa = true) {
        _c->clipRRect(rr, SkClipOp::kIntersect, aa); return *this;
    }
    // Per-corner clip: top-left, top-right, bottom-right, bottom-left
    AUKPainter& clipRoundedCorners(const SkRect& r,
                                   float tl, float tr, float br, float bl, bool aa = true) {
        const SkVector radii[4] = {{tl,tl},{tr,tr},{br,br},{bl,bl}};
        SkRRect rr; rr.setRectRadii(r, radii);
        _c->clipRRect(rr, SkClipOp::kIntersect, aa); return *this;
    }
    AUKPainter& clipCircle(float cx, float cy, float r, bool aa = true) {
        _c->clipRRect(
            SkRRect::MakeOval(SkRect::MakeXYWH(cx - r, cy - r, r * 2.f, r * 2.f)),
            SkClipOp::kIntersect, aa);
        return *this;
    }

    // ── Filled shapes ─────────────────────────────────────────────────────────
    AUKPainter& rect(const SkRect& r, AUKColor c) {
        _c->drawRect(r, fill(c)); return *this;
    }
    AUKPainter& roundRect(const SkRect& r, float radius, AUKColor c) {
        _c->drawRoundRect(r, radius, radius, fill(c)); return *this;
    }
    AUKPainter& roundRect(const SkRect& r, float rx, float ry, AUKColor c) {
        _c->drawRoundRect(r, rx, ry, fill(c)); return *this;
    }
    AUKPainter& rrect(const SkRRect& rr, AUKColor c) {
        _c->drawRRect(rr, fill(c)); return *this;
    }
    // Per-corner radii: top-left, top-right, bottom-right, bottom-left
    AUKPainter& roundRectCorners(const SkRect& r,
                                 float tl, float tr, float br, float bl, AUKColor c) {
        const SkVector radii[4] = {{tl,tl},{tr,tr},{br,br},{bl,bl}};
        SkRRect rr; rr.setRectRadii(r, radii);
        _c->drawRRect(rr, fill(c)); return *this;
    }
    AUKPainter& circle(float cx, float cy, float r, AUKColor c) {
        _c->drawCircle(cx, cy, r, fill(c)); return *this;
    }
    AUKPainter& path(const SkPath& path, AUKColor c) {
        _c->drawPath(path, fill(c)); return *this;
    }

    // ── Stroked shapes ────────────────────────────────────────────────────────
    AUKPainter& rectBorder(const SkRect& r, AUKColor c, float w = 1.f) {
        _c->drawRect(r, stroke(c, w)); return *this;
    }
    AUKPainter& roundRectBorder(const SkRect& r, float radius, AUKColor c, float w = 1.f) {
        _c->drawRoundRect(r, radius, radius, stroke(c, w)); return *this;
    }
    AUKPainter& rrectBorder(const SkRRect& rr, AUKColor c, float w = 1.f) {
        _c->drawRRect(rr, stroke(c, w)); return *this;
    }
    AUKPainter& circleBorder(float cx, float cy, float r, AUKColor c, float w = 1.f) {
        _c->drawCircle(cx, cy, r, stroke(c, w)); return *this;
    }
    AUKPainter& pathStroke(const SkPath& p, AUKColor c, float w = 1.f,
                           SkPaint::Cap cap = SkPaint::kRound_Cap) {
        auto pk = stroke(c, w); pk.setStrokeCap(cap);
        _c->drawPath(p, pk); return *this;
    }

    // ── Lines ─────────────────────────────────────────────────────────────────
    AUKPainter& line(float x1, float y1, float x2, float y2, AUKColor c, float w = 1.f,
                     SkPaint::Cap cap = SkPaint::kRound_Cap) {
        auto p = stroke(c, w); p.setStrokeCap(cap);
        _c->drawLine(x1, y1, x2, y2, p); return *this;
    }
    AUKPainter& line(SkPoint a, SkPoint b, AUKColor c, float w = 1.f) {
        _c->drawLine(a, b, stroke(c, w)); return *this;
    }

    // ── Arcs (degrees, 0 = 3 o'clock, clockwise) ─────────────────────────────
    AUKPainter& arc(const SkRect& oval, float startAngle, float sweepAngle,
                    AUKColor c, float w = 2.f,
                    SkPaint::Cap cap = SkPaint::kRound_Cap) {
        auto p = stroke(c, w); p.setStrokeCap(cap);
        _c->drawArc(oval, startAngle, sweepAngle, false, p); return *this;
    }
    // Filled wedge connected to centre
    AUKPainter& pie(const SkRect& oval, float startAngle, float sweepAngle, AUKColor c) {
        _c->drawArc(oval, startAngle, sweepAngle, true, fill(c)); return *this;
    }

    // ── Linear gradient ───────────────────────────────────────────────────────
    // angleDeg: 0 = left→right, 90 = top→bottom (default)
    AUKPainter& linearGradient(const SkRect& r, AUKColor from, AUKColor to,
                               float angleDeg = 90.f) {
        _c->drawRect(r, makeLinear(r, from, to, angleDeg)); return *this;
    }
    AUKPainter& roundRectGradient(const SkRect& r, float radius,
                                  AUKColor from, AUKColor to, float angleDeg = 90.f) {
        _c->drawRoundRect(r, radius, radius, makeLinear(r, from, to, angleDeg));
        return *this;
    }
    AUKPainter& circleGradient(float cx, float cy, float r,
                               AUKColor from, AUKColor to, float angleDeg = 90.f) {
        SkRect bounds = SkRect::MakeXYWH(cx - r, cy - r, r * 2.f, r * 2.f);
        _c->drawCircle(cx, cy, r, makeLinear(bounds, from, to, angleDeg));
        return *this;
    }

    // ── Radial gradient ───────────────────────────────────────────────────────
    // Draws a filled circle shading from centre colour to edge colour.
    AUKPainter& radialGradient(float cx, float cy, float r,
                               AUKColor center, AUKColor edge) {
        SkColor4f cols[2] = { toF(center), toF(edge) };
        SkGradient::Colors gc(SkSpan<const SkColor4f>(cols, 2), SkTileMode::kClamp);
        SkPaint p; p.setAntiAlias(true);
        p.setShader(SkShaders::RadialGradient(SkPoint{cx, cy}, r,
                        SkGradient(gc, SkGradient::Interpolation{})));
        _c->drawCircle(cx, cy, r, p); return *this;
    }
    // Fills a rect; radius = shortest half-extent of the rect.
    AUKPainter& radialGradientRect(const SkRect& r, AUKColor center, AUKColor edge) {
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
    AUKPainter& sweepGradient(float cx, float cy, float r,
                              AUKColor from, AUKColor to,
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
    AUKPainter& shadow(const SkRect& r, float cornerRadius, AUKColor c,
                       float sigma = 4.f, float dx = 0.f, float dy = 3.f) {
        SkPaint p; p.setAntiAlias(true);
        p.setColor(c.toSkColor());
        p.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma));
        _c->drawRoundRect(r.makeOffset(dx, dy), cornerRadius, cornerRadius, p);
        return *this;
    }
    AUKPainter& shadowCircle(float cx, float cy, float r, AUKColor c,
                             float sigma = 4.f, float dx = 0.f, float dy = 3.f) {
        SkPaint p; p.setAntiAlias(true);
        p.setColor(c.toSkColor());
        p.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma));
        _c->drawCircle(cx + dx, cy + dy, r, p); return *this;
    }

    // ── Escape hatch ──────────────────────────────────────────────────────────
    AUKPainter& withPaint(const SkPaint& paint,
                          void (*fn)(SkCanvas*, const SkPaint&)) {
        fn(_c, paint); return *this;
    }

private:
    SkCanvas* _c;

    static SkPaint fill(AUKColor c) {
        SkPaint p; p.setAntiAlias(true);
        p.setColor(c.toSkColor()); return p;
    }
    static SkPaint stroke(AUKColor c, float w) {
        SkPaint p; p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(w);
        p.setColor(c.toSkColor()); return p;
    }
    static SkColor4f toF(AUKColor c) {
        return { c.rf(), c.gf(), c.bf(), c.af() };
    }
    static SkPaint makeLinear(const SkRect& r, AUKColor from, AUKColor to, float angleDeg) {
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

} // namespace AureliaUI
