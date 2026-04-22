#include <include/gui/Components/TextNode.hpp>

namespace MochiUI {

Size TextNode::measure(Size available) {
    if (text.empty()) return { 0, 0 };

    SkRect bounds;
    FontManager::getInstance().measureText(text, fontSize, &bounds);

    float w = bounds.width() + 2 * style.padding;
    float h = bounds.height() + 2 * style.padding;

    if (style.widthMode == SizingMode::Fixed) w = style.width;
    if (style.heightMode == SizingMode::Fixed) h = style.height;

    return { w, h };
}

void TextNode::draw(SkCanvas* canvas) {
    // Draw hover effect only if enabled
    if (enableHover && isHovered) {
        SkPaint hoverPaint;
        hoverPaint.setAntiAlias(true);
        hoverPaint.setColor(Theme::HoverOverlay);
        canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, hoverPaint);
    }

    FlexNode::draw(canvas);
    if (!text.empty()) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(color);

        SkRect bounds;
        FontManager::getInstance().measureText(text, fontSize, &bounds);

        float x = frame.left() + (frame.width() - bounds.width()) / 2.0f - bounds.left();
        float y = frame.top() + (frame.height() - bounds.height()) / 2.0f - bounds.top();

        FontManager::getInstance().drawText(canvas, text, x, y, fontSize, paint);
    }
}
} // namespace MochiUI
