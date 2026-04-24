#include <include/gui/Components/TextNode.hpp>

namespace MochiUI {

Size TextNode::measure(Size available) {
    if (text.empty()) return { 0, 0 };
    SkRect bounds;
    FontManager::getInstance().measureText(text, fontSize, &bounds);
    // Use ceil to avoid sub-pixel cropping
    return { std::ceil(bounds.width()), std::ceil(bounds.height()) };
}

void TextNode::draw(SkCanvas* canvas) {
    drawSelf(canvas);

    if (enableHover && isHovered) {
        SkPaint hoverPaint;
        hoverPaint.setAntiAlias(true);
        hoverPaint.setColor(Theme::HoverOverlay);
        canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, hoverPaint);
    }

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

    drawChildren(canvas);
}
} // namespace MochiUI
