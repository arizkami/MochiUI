#include <gui/Components/SliderNode.hpp>
#include <algorithm>
#include <windows.h>

namespace SphereUI {

SliderNodeStyle SliderNode::resolveStyle() const {
    SliderNodeStyle resolved;
    resolved.trackColor = visualStyle.trackColor.value_or(trackColor);
    resolved.fillColor = visualStyle.fillColor.value_or(fillColor);
    resolved.thumbColor = visualStyle.thumbColor.value_or(thumbColor);
    resolved.thumbBorderColor = visualStyle.thumbBorderColor.value_or(SPHXColor(Theme::Background).withAlpha(uint8_t{100}));
    resolved.shadowColor = visualStyle.shadowColor.value_or(SPHXColor(Theme::Shadow).withAlpha(uint8_t{70}));
    resolved.trackHeight = visualStyle.trackHeight.value_or(trackHeight);
    resolved.thumbRadius = visualStyle.thumbRadius.value_or(thumbRadius);
    resolved.thumbBorderWidth = visualStyle.thumbBorderWidth.value_or(1.0f);
    return resolved;
}

Size SliderNode::measure(Size available) {
    const auto resolved = resolveStyle();
    const float thumbRadius = *resolved.thumbRadius;

    float w = vertical ? (thumbRadius * 2.0f + 20.0f) : 220.0f;
    float h = vertical ? 220.0f : (thumbRadius * 2.0f + 20.0f);

    if (style.widthMode == SizingMode::Fixed) w = style.width;
    if (style.heightMode == SizingMode::Fixed) h = style.height;

    return { w, h };
}

void SliderNode::draw(SkCanvas* canvas) {
    drawSelf(canvas);

    const auto resolved = resolveStyle();
    const SPHXColor trackColor = *resolved.trackColor;
    const SPHXColor fillColor = *resolved.fillColor;
    const SPHXColor thumbColor = *resolved.thumbColor;
    const SPHXColor thumbBorderColor = *resolved.thumbBorderColor;
    const SPHXColor shadowColor = *resolved.shadowColor;
    const float trackHeight = *resolved.trackHeight;
    const float layoutThumbR = *resolved.thumbRadius;
    const float drawThumbR   = layoutThumbR * (isDragging ? 1.08f : (isHovered ? 1.04f : 1.0f));
    const float thumbBorderWidth = *resolved.thumbBorderWidth;
    float norm = getNormalizedValue();

    if (isDragging || isHovered) {
        SkPaint halo;
        halo.setAntiAlias(true);
        halo.setColor(SkColorSetA(fillColor, isDragging ? 28 : 18));
        if (vertical) {
            const float hx = frame.centerX();
            const float trackTop = frame.top() + layoutThumbR;
            const float tH = frame.height() - 2 * layoutThumbR;
            const float hy = trackTop + tH - (tH * norm);
            canvas->drawCircle(hx, hy, drawThumbR + 5.0f, halo);
        } else {
            const float tX = frame.left() + layoutThumbR;
            const float tW = frame.width() - 2 * layoutThumbR;
            const float hx = tX + (tW * norm);
            const float hy = frame.centerY();
            canvas->drawCircle(hx, hy, drawThumbR + 5.0f, halo);
        }
    }

    if (vertical) {
        float trackX = frame.centerX() - trackHeight / 2.0f;
        float trackY = frame.top() + layoutThumbR;
        float trackW = trackHeight;
        float trackH = frame.height() - 2 * layoutThumbR;

        SkPaint trackPaint;
        trackPaint.setAntiAlias(true);
        trackPaint.setColor(trackColor);
        SkRRect trackRRect = SkRRect::MakeRectXY(
            SkRect::MakeXYWH(trackX, trackY, trackW, trackH),
            trackHeight / 2.0f, trackHeight / 2.0f
        );
        canvas->drawRRect(trackRRect, trackPaint);

        float fillH = trackH * norm;
        trackPaint.setColor(fillColor);
        SkRRect fillRRect = SkRRect::MakeRectXY(
            SkRect::MakeXYWH(trackX, trackY + trackH - fillH, trackW, fillH),
            trackHeight / 2.0f, trackHeight / 2.0f
        );
        canvas->drawRRect(fillRRect, trackPaint);

        float thumbY = trackY + trackH - (trackH * norm);
        SkPaint thumbPaint;
        thumbPaint.setAntiAlias(true);
        SPHXColor thCol = thumbColor;
        if (isHovered || isDragging)
            thCol = thCol.lighter(isDragging ? 0.14f : 0.08f);
        thumbPaint.setColor(thCol);

        SkPaint shadowPaint;
        shadowPaint.setAntiAlias(true);
        shadowPaint.setColor(shadowColor);
        canvas->drawCircle(frame.centerX(), thumbY + 2, drawThumbR, shadowPaint);

        canvas->drawCircle(frame.centerX(), thumbY, drawThumbR, thumbPaint);

        if (thumbBorderWidth > 0.0f) {
            SkPaint borderPaint;
            borderPaint.setAntiAlias(true);
            borderPaint.setColor(thumbBorderColor);
            borderPaint.setStyle(SkPaint::kStroke_Style);
            borderPaint.setStrokeWidth(thumbBorderWidth);
            canvas->drawCircle(frame.centerX(), thumbY, drawThumbR - (thumbBorderWidth * 0.5f), borderPaint);
        }

    } else {
        float trackX = frame.left() + layoutThumbR;
        float trackY = frame.centerY() - trackHeight / 2.0f;
        float trackW = frame.width() - 2 * layoutThumbR;
        float trackH = trackHeight;

        SkPaint trackPaint;
        trackPaint.setAntiAlias(true);
        trackPaint.setColor(trackColor);
        SkRRect trackRRect = SkRRect::MakeRectXY(
            SkRect::MakeXYWH(trackX, trackY, trackW, trackH),
            trackHeight / 2.0f, trackHeight / 2.0f
        );
        canvas->drawRRect(trackRRect, trackPaint);

        float fillW = trackW * norm;
        trackPaint.setColor(fillColor);
        SkRRect fillRRect = SkRRect::MakeRectXY(
            SkRect::MakeXYWH(trackX, trackY, fillW, trackH),
            trackHeight / 2.0f, trackHeight / 2.0f
        );
        canvas->drawRRect(fillRRect, trackPaint);

        float thumbX = trackX + (trackW * norm);
        SkPaint thumbPaint;
        thumbPaint.setAntiAlias(true);
        SPHXColor thCol = thumbColor;
        if (isHovered || isDragging)
            thCol = thCol.lighter(isDragging ? 0.14f : 0.08f);
        thumbPaint.setColor(thCol);

        SkPaint shadowPaint;
        shadowPaint.setAntiAlias(true);
        shadowPaint.setColor(shadowColor);
        canvas->drawCircle(thumbX, frame.centerY() + 2, drawThumbR, shadowPaint);

        canvas->drawCircle(thumbX, frame.centerY(), drawThumbR, thumbPaint);

        if (thumbBorderWidth > 0.0f) {
            SkPaint borderPaint;
            borderPaint.setAntiAlias(true);
            borderPaint.setColor(thumbBorderColor);
            borderPaint.setStyle(SkPaint::kStroke_Style);
            borderPaint.setStrokeWidth(thumbBorderWidth);
            canvas->drawCircle(thumbX, frame.centerY(), drawThumbR - (thumbBorderWidth * 0.5f), borderPaint);
        }
    }

    drawChildren(canvas);
}

bool SliderNode::onMouseDown(float x, float y) {
    if (hitTest(x, y)) {
        isDragging = true;
        updateValueFromPosition(x, y);
        return true;
    }
    return false;
}

bool SliderNode::onMouseMove(float x, float y) {
    bool handled = FlexNode::onMouseMove(x, y);
    if (isDragging) {
        updateValueFromPosition(x, y);
        return true;
    }
    return handled || isHovered;
}

void SliderNode::onMouseUp(float x, float y) {
    isDragging = false;
    FlexNode::onMouseUp(x, y);
}

bool SliderNode::onMouseWheel(float x, float y, float delta) {
    if (hitTest(x, y)) {
        bool shiftPressed = GetKeyState(VK_SHIFT) & 0x8000;
        float sensitivity = shiftPressed ? 0.01f : 0.05f;

        float norm = getNormalizedValue();
        norm += (delta > 0 ? sensitivity : -sensitivity);
        norm = std::clamp(norm, 0.0f, 1.0f);

        value = minValue + norm * (maxValue - minValue);

        if (onValueChange) {
            onValueChange(value);
        }
        return true;
    }
    return false;
}

void SliderNode::updateValueFromPosition(float x, float y) {
    const auto resolved = resolveStyle();
    const float thumbRadius = *resolved.thumbRadius;
    float norm;

    if (vertical) {
        float trackY = frame.top() + thumbRadius;
        float trackH = frame.height() - 2 * thumbRadius;
        norm = 1.0f - std::clamp((y - trackY) / trackH, 0.0f, 1.0f);
    } else {
        float trackX = frame.left() + thumbRadius;
        float trackW = frame.width() - 2 * thumbRadius;
        norm = std::clamp((x - trackX) / trackW, 0.0f, 1.0f);
    }

    value = minValue + norm * (maxValue - minValue);

    if (onValueChange) {
        onValueChange(value);
    }
}

float SliderNode::getNormalizedValue() const {
    return std::clamp((value - minValue) / (maxValue - minValue), 0.0f, 1.0f);
}

} // namespace SphereUI
