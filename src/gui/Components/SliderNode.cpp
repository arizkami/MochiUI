#include <gui/Components/SliderNode.hpp>
#include <algorithm>
#include <windows.h>

namespace MochiUI {

Size SliderNode::measure(Size available) {
    float w = vertical ? 40.0f : 200.0f;
    float h = vertical ? 200.0f : 40.0f;
    
    if (style.widthMode == SizingMode::Fixed) w = style.width;
    if (style.heightMode == SizingMode::Fixed) h = style.height;
    
    return { w, h };
}

void SliderNode::draw(SkCanvas* canvas) {
    drawSelf(canvas);
    
    float norm = getNormalizedValue();
    
    if (vertical) {
        // Vertical slider
        float trackX = frame.centerX() - trackHeight / 2.0f;
        float trackY = frame.top() + thumbRadius;
        float trackW = trackHeight;
        float trackH = frame.height() - 2 * thumbRadius;
        
        // Draw track background
        SkPaint trackPaint;
        trackPaint.setAntiAlias(true);
        trackPaint.setColor(trackColor);
        SkRRect trackRRect = SkRRect::MakeRectXY(
            SkRect::MakeXYWH(trackX, trackY, trackW, trackH),
            trackHeight / 2.0f, trackHeight / 2.0f
        );
        canvas->drawRRect(trackRRect, trackPaint);
        
        // Draw filled portion (from bottom)
        float fillH = trackH * norm;
        trackPaint.setColor(fillColor);
        SkRRect fillRRect = SkRRect::MakeRectXY(
            SkRect::MakeXYWH(trackX, trackY + trackH - fillH, trackW, fillH),
            trackHeight / 2.0f, trackHeight / 2.0f
        );
        canvas->drawRRect(fillRRect, trackPaint);
        
        // Draw thumb
        float thumbY = trackY + trackH - (trackH * norm);
        SkPaint thumbPaint;
        thumbPaint.setAntiAlias(true);
        thumbPaint.setColor(thumbColor);
        canvas->drawCircle(frame.centerX(), thumbY, thumbRadius, thumbPaint);
        
        // Thumb shadow
        thumbPaint.setColor(SkColorSetARGB(40, 0, 0, 0));
        canvas->drawCircle(frame.centerX(), thumbY + 1, thumbRadius, thumbPaint);
        
    } else {
        // Horizontal slider
        float trackX = frame.left() + thumbRadius;
        float trackY = frame.centerY() - trackHeight / 2.0f;
        float trackW = frame.width() - 2 * thumbRadius;
        float trackH = trackHeight;
        
        // Draw track background
        SkPaint trackPaint;
        trackPaint.setAntiAlias(true);
        trackPaint.setColor(trackColor);
        SkRRect trackRRect = SkRRect::MakeRectXY(
            SkRect::MakeXYWH(trackX, trackY, trackW, trackH),
            trackHeight / 2.0f, trackHeight / 2.0f
        );
        canvas->drawRRect(trackRRect, trackPaint);
        
        // Draw filled portion
        float fillW = trackW * norm;
        trackPaint.setColor(fillColor);
        SkRRect fillRRect = SkRRect::MakeRectXY(
            SkRect::MakeXYWH(trackX, trackY, fillW, trackH),
            trackHeight / 2.0f, trackHeight / 2.0f
        );
        canvas->drawRRect(fillRRect, trackPaint);
        
        // Draw thumb
        float thumbX = trackX + (trackW * norm);
        SkPaint thumbPaint;
        thumbPaint.setAntiAlias(true);
        thumbPaint.setColor(thumbColor);
        canvas->drawCircle(thumbX, frame.centerY(), thumbRadius, thumbPaint);
        
        // Thumb shadow
        thumbPaint.setColor(SkColorSetARGB(40, 0, 0, 0));
        canvas->drawCircle(thumbX + 1, frame.centerY() + 1, thumbRadius, thumbPaint);
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
    if (isDragging) {
        updateValueFromPosition(x, y);
        return true;
    }
    return false;
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

} // namespace MochiUI
