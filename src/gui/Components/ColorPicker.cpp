#include <include/gui/Components/ColorPicker.hpp>
#include <include/core/SkShader.h>
#include <include/effects/SkGradient.h>
#include <include/effects/SkRuntimeEffect.h>
#include <algorithm>

namespace MochiUI {

ColorPicker::ColorPicker() {
    style.setWidth(220.0f);
    style.setHeight(200.0f);
    style.setPadding(10.0f);
    updateFromHSV();
}

void ColorPicker::setColor(SkColor color) {
    currentColor = color;
    // Simple RGB to HSV conversion
    float r = SkColorGetR(color) / 255.0f;
    float g = SkColorGetG(color) / 255.0f;
    float b = SkColorGetB(color) / 255.0f;
    float max = std::max({r, g, b});
    float min = std::min({r, g, b});
    float d = max - min;
    v = max;
    s = max == 0 ? 0 : d / max;
    if (max == min) {
        h = 0;
    } else {
        if (max == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (max == g) h = (b - r) / d + 2;
        else if (max == b) h = (r - g) / d + 4;
        h /= 6;
    }
}

void ColorPicker::updateFromHSV() {
    float r, g, b;
    int i = (int)(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
    currentColor = SkColorSetRGB((U8CPU)(r * 255), (U8CPU)(g * 255), (U8CPU)(b * 255));
    if (onColorChanged) onColorChanged(currentColor);
}

Size ColorPicker::measure(Size available) {
    return { 220.0f, 200.0f };
}

void ColorPicker::draw(SkCanvas* canvas) {
    drawSelf(canvas);

    float padding = 10.0f;
    float hueWidth = 20.0f;
    SkRect svRect = SkRect::MakeXYWH(frame.left() + padding, frame.top() + padding, 
                                     frame.width() - 3 * padding - hueWidth, 
                                     frame.height() - 2 * padding);
    SkRect hRect = SkRect::MakeXYWH(svRect.right() + padding, frame.top() + padding, 
                                    hueWidth, frame.height() - 2 * padding);

    // Draw SV Box
    SkColor hColor;
    {
        float r, g, b;
        float hue = h;
        int i = (int)(hue * 6);
        float f = hue * 6 - i;
        float v_val = 1, s_val = 1, p = 0, q = 1-f, t = f;
        switch (i % 6) {
            case 0: r = v_val, g = t, b = p; break;
            case 1: r = q, g = v_val, b = p; break;
            case 2: r = p, g = v_val, b = t; break;
            case 3: r = p, g = q, b = v_val; break;
            case 4: r = t, g = p, b = v_val; break;
            case 5: r = v_val, g = p, b = q; break;
        }
        hColor = SkColorSetRGB((U8CPU)(r * 255), (U8CPU)(g * 255), (U8CPU)(b * 255));
    }

    // New Skia Gradient API
    {
        SkPoint pts[2] = { {svRect.left(), svRect.top()}, {svRect.right(), svRect.top()} };
        SkColor4f colorsW[2] = { SkColor4f::FromColor(SK_ColorWHITE), SkColor4f::FromColor(hColor) };
        SkGradient grad(SkGradient::Colors(colorsW, SkTileMode::kClamp), {});
        
        SkPaint svPaint;
        svPaint.setShader(SkShaders::LinearGradient(pts, grad));
        canvas->drawRect(svRect, svPaint);
    }

    {
        SkPoint ptsV[2] = { {svRect.left(), svRect.top()}, {svRect.left(), svRect.bottom()} };
        SkColor4f colorsV[2] = { SkColor4f::FromColor(SK_ColorWHITE), SkColor4f::FromColor(SK_ColorBLACK) };
        SkGradient gradV(SkGradient::Colors(colorsV, SkTileMode::kClamp), {});

        SkPaint vPaint;
        vPaint.setBlendMode(SkBlendMode::kMultiply);
        vPaint.setShader(SkShaders::LinearGradient(ptsV, gradV));
        canvas->drawRect(svRect, vPaint);
    }

    // Draw SV cursor
    SkPaint cursorPaint;
    cursorPaint.setAntiAlias(true);
    cursorPaint.setStyle(SkPaint::kStroke_Style);
    cursorPaint.setStrokeWidth(2.0f);
    cursorPaint.setColor(v > 0.5f ? SK_ColorBLACK : SK_ColorWHITE);
    canvas->drawCircle(svRect.left() + s * svRect.width(), 
                       svRect.top() + (1.0f - v) * svRect.height(), 4.0f, cursorPaint);

    // Draw Hue Bar
    {
        SkPoint ptsH[2] = { {hRect.left(), hRect.top()}, {hRect.left(), hRect.bottom()} };
        SkColor4f hColors[7] = { 
            SkColor4f::FromColor(0xFFFF0000), 
            SkColor4f::FromColor(0xFFFFFF00), 
            SkColor4f::FromColor(0xFF00FF00), 
            SkColor4f::FromColor(0xFF00FFFF), 
            SkColor4f::FromColor(0xFF0000FF), 
            SkColor4f::FromColor(0xFFFF00FF), 
            SkColor4f::FromColor(0xFFFF0000) 
        };
        SkGradient gradH(SkGradient::Colors(hColors, SkTileMode::kClamp), {});
        
        SkPaint hPaint;
        hPaint.setShader(SkShaders::LinearGradient(ptsH, gradH));
        canvas->drawRect(hRect, hPaint);
    }

    // Draw Hue cursor
    SkPaint hCursorPaint;
    hCursorPaint.setColor(SK_ColorBLACK);
    hCursorPaint.setStyle(SkPaint::kStroke_Style);
    hCursorPaint.setStrokeWidth(2.0f);
    canvas->drawRect(SkRect::MakeXYWH(hRect.left() - 2, hRect.top() + h * hRect.height() - 2, 
                                      hRect.width() + 4, 4), hCursorPaint);
}

bool ColorPicker::onMouseDown(float x, float y) {
    if (!hitTest(x, y)) return false;
    updateFromXY(x, y);
    return true;
}

bool ColorPicker::onMouseMove(float x, float y) {
    if (draggingSV || draggingH) {
        updateFromXY(x, y);
        return true;
    }
    return FlexNode::onMouseMove(x, y);
}

void ColorPicker::onMouseUp(float x, float y) {
    draggingSV = false;
    draggingH = false;
    FlexNode::onMouseUp(x, y);
}

void ColorPicker::updateFromXY(float x, float y) {
    float padding = 10.0f;
    float hueWidth = 20.0f;
    SkRect svRect = SkRect::MakeXYWH(frame.left() + padding, frame.top() + padding, 
                                     frame.width() - 3 * padding - hueWidth, 
                                     frame.height() - 2 * padding);
    SkRect hRect = SkRect::MakeXYWH(svRect.right() + padding, frame.top() + padding, 
                                    hueWidth, frame.height() - 2 * padding);

    if (hRect.contains(x, y) || draggingH) {
        draggingH = true;
        h = std::clamp((y - hRect.top()) / hRect.height(), 0.0f, 1.0f);
        updateFromHSV();
    } else if (svRect.contains(x, y) || draggingSV) {
        draggingSV = true;
        s = std::clamp((x - svRect.left()) / svRect.width(), 0.0f, 1.0f);
        v = 1.0f - std::clamp((y - svRect.top()) / svRect.height(), 0.0f, 1.0f);
        updateFromHSV();
    }
}

} // namespace MochiUI
