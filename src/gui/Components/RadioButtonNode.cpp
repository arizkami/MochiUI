#include <include/gui/Components/RadioButtonNode.hpp>

namespace MochiUI {

Size RadioButtonNode::measure(Size available) {
    float labelWidth = 0;
    float labelHeight = fontSize;

    if (!label.empty()) {
        SkRect bounds;
        FontManager::getInstance().measureText(label, fontSize, &bounds);
        labelWidth = bounds.width();
        labelHeight = bounds.height();
    }

    float totalWidth = radioSize + (label.empty() ? 0 : (spacing + labelWidth));
    float totalHeight = std::max(radioSize, labelHeight);

    return { std::ceil(totalWidth), std::ceil(totalHeight) };
}

void RadioButtonNode::draw(SkCanvas* canvas) {
    if (!canvas) return;

    drawSelf(canvas);

    float radioX = frame.left() + style.padding;
    float radioY = frame.centerY() - radioSize / 2;

    float centerX = radioX + radioSize / 2;
    float centerY = radioY + radioSize / 2;
    float outerRadius = radioSize / 2;

    SkPaint outerPaint;
    outerPaint.setAntiAlias(true);
    outerPaint.setStyle(SkPaint::kStroke_Style);
    outerPaint.setStrokeWidth(2.0f);
    outerPaint.setColor(isHovered ? radioColor : SkColorSetA(radioColor, 180));

    canvas->drawCircle(centerX, centerY, outerRadius - 1.0f, outerPaint);

    if (selected) {
        SkPaint innerPaint;
        innerPaint.setAntiAlias(true);
        innerPaint.setStyle(SkPaint::kFill_Style);
        innerPaint.setColor(radioColor);

        canvas->drawCircle(centerX, centerY, outerRadius - 5.0f, innerPaint);
    }

    if (!label.empty()) {
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(labelColor);

        SkRect bounds;
        FontManager::getInstance().measureText(label, fontSize, &bounds);

        float textX = radioX + radioSize + spacing;
        float textY = frame.centerY() - bounds.centerY();

        FontManager::getInstance().drawText(canvas, label, textX, textY, fontSize, textPaint);
    }

    drawChildren(canvas);
}

bool RadioButtonNode::onMouseDown(float x, float y) {
    if (hitTest(x, y)) {
        if (!selected) {
            selected = true;
            if (onSelected) {
                onSelected(true);
            }
        }
        return true;
    }
    return FlexNode::onMouseDown(x, y);
}

} // namespace MochiUI
