#include <include/gui/Components/SwitchNode.hpp>
#include <include/core/SkRRect.h>

namespace MochiUI {

Size SwitchNode::measure(Size available) {
    float labelWidth = 0;
    float labelHeight = fontSize;

    if (!label.empty()) {
        SkRect bounds;
        FontManager::getInstance().measureText(label, fontSize, &bounds);
        labelWidth = bounds.width();
        labelHeight = bounds.height();
    }

    float totalWidth = switchWidth + (label.empty() ? 0 : (spacing + labelWidth));
    float totalHeight = std::max(switchHeight, labelHeight);

    return { std::ceil(totalWidth), std::ceil(totalHeight) };
}

void SwitchNode::draw(SkCanvas* canvas) {
    if (!canvas) return;

    drawSelf(canvas);

    float trackX = frame.left() + getLayoutPadding(YGEdgeLeft);
    float trackY = frame.centerY() - switchHeight / 2;

    SkRect trackRect = SkRect::MakeXYWH(trackX, trackY, switchWidth, switchHeight);
    float radius = switchHeight / 2.0f;

    SkPaint trackPaint;
    trackPaint.setAntiAlias(true);
    trackPaint.setColor(isOn ? activeColor : inactiveColor);
    
    canvas->drawRoundRect(trackRect, radius, radius, trackPaint);

    // Draw Thumb
    float thumbMargin = 3.0f;
    float thumbSize = switchHeight - (thumbMargin * 2);
    float thumbX = isOn ? (trackRect.right() - thumbSize - thumbMargin) : (trackRect.left() + thumbMargin);
    float thumbY = trackRect.top() + thumbMargin;

    SkRect thumbRect = SkRect::MakeXYWH(thumbX, thumbY, thumbSize, thumbSize);
    SkPaint thumbPaint;
    thumbPaint.setAntiAlias(true);
    thumbPaint.setColor(thumbColor);
    
    canvas->drawRoundRect(thumbRect, thumbSize / 2, thumbSize / 2, thumbPaint);

    if (!label.empty()) {
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(labelColor);

        SkRect bounds;
        FontManager::getInstance().measureText(label, fontSize, &bounds);

        float textX = trackX + switchWidth + spacing;
        float textY = frame.centerY() - bounds.centerY();

        FontManager::getInstance().drawText(canvas, label, textX, textY, fontSize, textPaint);
    }

    drawChildren(canvas);
}

bool SwitchNode::onMouseDown(float x, float y) {
    if (hitTest(x, y)) {
        isOn = !isOn;
        if (onChanged) {
            onChanged(isOn);
        }
        return true;
    }
    return FlexNode::onMouseDown(x, y);
}

} // namespace MochiUI
