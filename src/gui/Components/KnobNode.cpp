#include <include/gui/Components/KnobNode.hpp>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace MochiUI {

Size KnobNode::measure(Size available) {
    float size = knobSize + 20.0f;  // Add padding for arc
    
    if (style.widthMode == SizingMode::Fixed) size = style.width;
    if (style.heightMode == SizingMode::Fixed) size = style.height;
    
    return { size, size };
}

void KnobNode::draw(SkCanvas* canvas) {
    FlexNode::draw(canvas);
    
    float centerX = frame.centerX();
    float centerY = frame.centerY();
    float radius = knobSize / 2.0f;
    float norm = getNormalizedValue();
    
    // Draw outer shadow
    SkPaint shadowPaint;
    shadowPaint.setAntiAlias(true);
    shadowPaint.setColor(SkColorSetARGB(50, 0, 0, 0));
    canvas->drawCircle(centerX + 2, centerY + 3, radius + 2, shadowPaint);
    
    // Draw outer ring
    SkPaint ringPaint;
    ringPaint.setAntiAlias(true);
    ringPaint.setColor(knobRingColor);
    canvas->drawCircle(centerX, centerY, radius, ringPaint);
    
    // Draw inner knob body with gradient effect
    SkPaint knobPaint;
    knobPaint.setAntiAlias(true);
    knobPaint.setColor(knobBodyColor);
    canvas->drawCircle(centerX, centerY, radius - 4, knobPaint);
    
    // Draw subtle highlight on top
    SkPaint highlightPaint;
    highlightPaint.setAntiAlias(true);
    highlightPaint.setColor(SkColorSetARGB(20, 255, 255, 255));
    canvas->drawCircle(centerX, centerY - radius * 0.3f, radius * 0.4f, highlightPaint);
    
    // Draw arc track (background)
    SkPaint arcPaint;
    arcPaint.setAntiAlias(true);
    arcPaint.setStyle(SkPaint::kStroke_Style);
    arcPaint.setStrokeWidth(arcWidth);
    arcPaint.setStrokeCap(SkPaint::kRound_Cap);
    arcPaint.setColor(arcTrackColor);
    
    float arcRadius = radius + 10;
    SkRect arcRect = SkRect::MakeXYWH(
        centerX - arcRadius,
        centerY - arcRadius,
        arcRadius * 2,
        arcRadius * 2
    );
    
    canvas->drawArc(arcRect, startAngle, sweepAngle, false, arcPaint);
    
    // Draw arc fill (value indicator)
    arcPaint.setColor(arcFillColor);
    arcPaint.setStrokeWidth(arcWidth + 1);
    canvas->drawArc(arcRect, startAngle, sweepAngle * norm, false, arcPaint);
    
    // Draw tick marks
    SkPaint tickPaint;
    tickPaint.setAntiAlias(true);
    tickPaint.setColor(SkColorSetARGB(100, 255, 255, 255));
    tickPaint.setStrokeWidth(1.5f);
    
    for (int i = 0; i <= 10; i++) {
        float tickAngle = startAngle + (sweepAngle * i / 10.0f);
        float tickRad = tickAngle * M_PI / 180.0f;
        float tickStart = radius + 6;
        float tickEnd = radius + 9;
        
        float x1 = centerX + std::cos(tickRad) * tickStart;
        float y1 = centerY + std::sin(tickRad) * tickStart;
        float x2 = centerX + std::cos(tickRad) * tickEnd;
        float y2 = centerY + std::sin(tickRad) * tickEnd;
        
        canvas->drawLine(x1, y1, x2, y2, tickPaint);
    }
    
    // Draw value indicator dot
    float angle = getAngleForValue();
    float angleRad = angle * M_PI / 180.0f;
    float dotRadius = radius - 10;
    
    float dotX = centerX + std::cos(angleRad) * dotRadius;
    float dotY = centerY + std::sin(angleRad) * dotRadius;
    
    SkPaint dotPaint;
    dotPaint.setAntiAlias(true);
    dotPaint.setColor(indicatorColor);
    canvas->drawCircle(dotX, dotY, 4.0f, dotPaint);
    
    // Draw glow around indicator dot
    dotPaint.setColor(SkColorSetARGB(60, SkColorGetR(indicatorColor), 
                                      SkColorGetG(indicatorColor), 
                                      SkColorGetB(indicatorColor)));
    canvas->drawCircle(dotX, dotY, 6.0f, dotPaint);
}

bool KnobNode::onMouseDown(float x, float y) {
    if (hitTest(x, y)) {
        isDragging = true;
        lastMouseY = y;
        return true;
    }
    return false;
}

bool KnobNode::onMouseMove(float x, float y) {
    if (isDragging) {
        updateValueFromPosition(x, y);
        return true;
    }
    return false;
}

void KnobNode::onMouseUp(float x, float y) {
    isDragging = false;
    FlexNode::onMouseUp(x, y);
}

void KnobNode::updateValueFromPosition(float x, float y) {
    // Vertical drag to change value
    float delta = (lastMouseY - y) * 0.005f;  // Sensitivity
    lastMouseY = y;
    
    float norm = getNormalizedValue() + delta;
    norm = std::clamp(norm, 0.0f, 1.0f);
    
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
