#include <gui/Components/KnobNode.hpp>
#include <algorithm>
#include <windows.h> // For GetTickCount and GetKeyState

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace MochiUI {

Size KnobNode::measure(Size available) {
    float size = knobSize + 30.0f;  // Increased padding for larger arc and effects
    
    if (style.widthMode == SizingMode::Fixed) size = style.width;
    if (style.heightMode == SizingMode::Fixed) size = style.height;
    
    return { size, size };
}

void KnobNode::draw(SkCanvas* canvas) {
    float centerX = frame.centerX();
    float centerY = frame.centerY();
    float radius = knobSize / 2.0f;
    float norm = getNormalizedValue();
    
    // 1. Draw Outer Glow (if dragging or hovered)
    if (isDragging || isHovered) {
        SkPaint glowPaint;
        glowPaint.setAntiAlias(true);
        glowPaint.setColor(SkColorSetA(arcFillColor, isDragging ? 40 : 20));
        canvas->drawCircle(centerX, centerY, radius + 15, glowPaint);
    }

    // 2. Draw Arc Track (Background)
    SkPaint arcPaint;
    arcPaint.setAntiAlias(true);
    arcPaint.setStyle(SkPaint::kStroke_Style);
    arcPaint.setStrokeWidth(arcWidth);
    arcPaint.setStrokeCap(SkPaint::kRound_Cap);
    arcPaint.setColor(arcTrackColor);
    
    float arcRadius = radius + 8;
    SkRect arcRect = SkRect::MakeXYWH(
        centerX - arcRadius,
        centerY - arcRadius,
        arcRadius * 2,
        arcRadius * 2
    );
    canvas->drawArc(arcRect, startAngle, sweepAngle, false, arcPaint);
    
    // 3. Draw Arc Fill (Value)
    arcPaint.setColor(arcFillColor);
    arcPaint.setStrokeWidth(arcWidth + 1);
    // Add a slight glow to the arc itself
    if (isDragging) {
        arcPaint.setAlphaf(1.0f);
    }
    canvas->drawArc(arcRect, startAngle, sweepAngle * norm, false, arcPaint);
    
    // 4. Draw Knob Body Shadow
    SkPaint shadowPaint;
    shadowPaint.setAntiAlias(true);
    shadowPaint.setColor(SkColorSetARGB(80, 0, 0, 0));
    canvas->drawCircle(centerX, centerY + 3, radius, shadowPaint);

    // 5. Draw Knob Outer Ring
    SkPaint ringPaint;
    ringPaint.setAntiAlias(true);
    ringPaint.setColor(knobRingColor);
    canvas->drawCircle(centerX, centerY, radius, ringPaint);
    
    // 6. Draw Knob Face (Gradient)
    SkPaint facePaint;
    facePaint.setAntiAlias(true);
    facePaint.setColor(knobBodyColor);
    canvas->drawCircle(centerX, centerY, radius - 3, facePaint);

    // 7. Draw Indicator Line (instead of dot, for more professional look)
    float angle = getAngleForValue();
    float angleRad = angle * M_PI / 180.0f;
    
    SkPaint indicatorPaint;
    indicatorPaint.setAntiAlias(true);
    indicatorPaint.setColor(indicatorColor);
    indicatorPaint.setStrokeWidth(3.0f);
    indicatorPaint.setStrokeCap(SkPaint::kRound_Cap);
    
    float innerR = radius * 0.4f;
    float outerR = radius - 8.0f;
    
    float x1 = centerX + std::cos(angleRad) * innerR;
    float y1 = centerY + std::sin(angleRad) * innerR;
    float x2 = centerX + std::cos(angleRad) * outerR;
    float y2 = centerY + std::sin(angleRad) * outerR;
    
    canvas->drawLine(x1, y1, x2, y2, indicatorPaint);
    
    // 8. Draw Top Highlight / Cap Effect
    SkPaint capPaint;
    capPaint.setAntiAlias(true);
    capPaint.setColor(SkColorSetARGB(30, 255, 255, 255));
    canvas->drawCircle(centerX - radius * 0.2f, centerY - radius * 0.2f, radius * 0.3f, capPaint);
}

bool KnobNode::hitTest(float x, float y) {
    float dx = x - frame.centerX();
    float dy = y - frame.centerY();
    float radius = knobSize / 2.0f;
    // Allow a slight margin for the outer arc track
    float hitRadius = radius + 15.0f; 
    return (dx * dx + dy * dy) <= (hitRadius * hitRadius);
}

bool KnobNode::onMouseDown(float x, float y) {
    if (hitTest(x, y)) {
        uint32_t currentTime = GetTickCount();
        if (currentTime - lastClickTime < 300) { // Double click detection
            onDoubleClick(x, y);
            isDragging = false;
            return true;
        }
        lastClickTime = currentTime;
        
        // Reset to default on Alt+Click
        if (GetKeyState(VK_MENU) & 0x8000) {
            value = defaultValue;
            if (onValueChange) onValueChange(value);
            return true;
        }

        isDragging = true;
        lastMouseY = y;
        
        // Set value immediately on click (rotational)
        updateValueFromRotation(x, y);
        
        return true;
    }
    return false;
}

bool KnobNode::onMouseMove(float x, float y) {
    bool handled = FlexNode::onMouseMove(x, y); // For hover state
    if (isDragging) {
        updateValueFromRotation(x, y);
        return true;
    }
    return handled || isHovered; // Request redraw on hover
}

void KnobNode::onMouseUp(float x, float y) {
    isDragging = false;
    FlexNode::onMouseUp(x, y);
}

bool KnobNode::onMouseWheel(float x, float y, float delta) {
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

bool KnobNode::onDoubleClick(float x, float y) {
    value = defaultValue;
    if (onValueChange) onValueChange(value);
    return true;
}

void KnobNode::updateValueFromPosition(float x, float y) {
    bool shiftPressed = GetKeyState(VK_SHIFT) & 0x8000;
    float sensitivity = shiftPressed ? 0.001f : 0.005f;
    
    float delta = (lastMouseY - y) * sensitivity;
    lastMouseY = y;
    
    float norm = getNormalizedValue() + delta;
    norm = std::clamp(norm, 0.0f, 1.0f);
    
    value = minValue + norm * (maxValue - minValue);
    
    if (onValueChange) {
        onValueChange(value);
    }
}

void KnobNode::updateValueFromRotation(float x, float y) {
    float dx = x - frame.centerX();
    float dy = y - frame.centerY();
    
    // Avoid erratic behavior near center
    if (std::abs(dx) < 2 && std::abs(dy) < 2) return;

    float angle = std::atan2(dy, dx) * 180.0f / M_PI; // -180 to 180
    
    float normAngle = angle;
    if (normAngle < 0) normAngle += 360.0f;
    
    // Shift so startAngle is 0
    float relativeAngle = normAngle - startAngle;
    if (relativeAngle < 0) relativeAngle += 360.0f;
    
    float norm = 0.0f;
    if (relativeAngle <= sweepAngle) {
        norm = relativeAngle / sweepAngle;
    } else {
        // In the gap, snap to nearest end
        float gapCenter = sweepAngle + (360.0f - sweepAngle) / 2.0f;
        norm = (relativeAngle > gapCenter) ? 0.0f : 1.0f;
    }
    
    value = minValue + norm * (maxValue - minValue);
    
    if (onValueChange) {
        onValueChange(value);
    }
}

float KnobNode::getNormalizedValue() const {
    return std::clamp((value - minValue) / (maxValue - minValue), 0.0f, 1.0f);
}

float KnobNode::getAngleForValue() const {
    float norm = getNormalizedValue();
    return startAngle + (sweepAngle * norm);
}

} // namespace MochiUI
