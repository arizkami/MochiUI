#include <include/gui/Components/CheckboxNode.hpp>

namespace MochiUI {

Size CheckboxNode::measure(Size available) {
    float labelWidth = 0;
    float labelHeight = fontSize;

    if (!label.empty()) {
        SkRect bounds;
        FontManager::getInstance().measureText(label, fontSize, &bounds);
        labelWidth = bounds.width();
        labelHeight = bounds.height();
    }

    // Yoga adds padding automatically based on style, 
    // so we should only return the intrinsic content size here.
    float totalWidth = checkboxSize + (label.empty() ? 0 : (spacing + labelWidth));
    float totalHeight = std::max(checkboxSize, labelHeight);

    return { std::ceil(totalWidth), std::ceil(totalHeight) };
}

void CheckboxNode::draw(SkCanvas* canvas) {
    if (!canvas) return;

    drawSelf(canvas);

    float checkboxX = frame.left() + getLayoutPadding(YGEdgeLeft);
    float checkboxY = frame.centerY() - checkboxSize / 2;

    SkRect checkboxRect = SkRect::MakeXYWH(checkboxX, checkboxY, checkboxSize, checkboxSize);

    SkPaint boxPaint;
    boxPaint.setAntiAlias(true);
    boxPaint.setStyle(SkPaint::kStroke_Style);
    boxPaint.setStrokeWidth(Theme::BorderWidth);
    boxPaint.setColor(checked ? checkboxColor : (isHovered ? checkboxColor : Theme::Border));

    float radius = std::min(4.0f, Theme::BorderRadius);
    canvas->drawRoundRect(checkboxRect, radius, radius, boxPaint);

    if (checked) {
        SkPaint fillPaint;
        fillPaint.setAntiAlias(true);
        fillPaint.setColor(checkboxColor);

        SkRect innerRect = checkboxRect;
        innerRect.inset(3, 3);
        canvas->drawRoundRect(innerRect, radius - 1, radius - 1, fillPaint);

        SkPaint checkPaint;
        checkPaint.setAntiAlias(true);
        checkPaint.setStyle(SkPaint::kStroke_Style);
        checkPaint.setStrokeWidth(2.0f);
        checkPaint.setColor(SK_ColorWHITE);
        checkPaint.setStrokeCap(SkPaint::kRound_Cap);

        float cx = checkboxRect.centerX();
        float cy = checkboxRect.centerY();
        float size = checkboxSize * 0.3f;

        // Draw checkmark using lines
        canvas->drawLine(cx - size * 0.5f, cy,
                        cx - size * 0.1f, cy + size * 0.5f, checkPaint);
        canvas->drawLine(cx - size * 0.1f, cy + size * 0.5f,
                        cx + size * 0.6f, cy - size * 0.4f, checkPaint);
    }

    if (!label.empty()) {
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(labelColor);

        SkRect bounds;
        FontManager::getInstance().measureText(label, fontSize, &bounds);

        float textX = checkboxX + checkboxSize + spacing;
        float textY = frame.centerY() - bounds.centerY();

        FontManager::getInstance().drawText(canvas, label, textX, textY, fontSize, textPaint);
    }

    drawChildren(canvas);
}
bool CheckboxNode::onMouseDown(float x, float y) {
    if (hitTest(x, y)) {
        checked = !checked;
        if (onChanged) {
            onChanged(checked);
        }
        return true;
    }
    return FlexNode::onMouseDown(x, y);
}

} // namespace MochiUI
