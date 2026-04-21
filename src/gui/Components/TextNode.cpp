#include <include/gui/Components/TextNode.hpp>

namespace MochiUI {

Size TextNode::measure(Size available) {
    if (text.empty()) return { 0, 0 };
    
    SkFont font = FontManager::getInstance().createFont(
        FontManager::DEFAULT_FONT, fontSize);
    SkRect bounds;
    font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8, &bounds);
    
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
        SkFont font = FontManager::getInstance().createFont(
            FontManager::DEFAULT_FONT, fontSize);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(color);
        
        SkRect bounds;
        font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8, &bounds);
        
        float x = frame.left() + (frame.width() - bounds.width()) / 2.0f - bounds.left();
        float y = frame.top() + (frame.height() - bounds.height()) / 2.0f - bounds.top();
        
        canvas->drawSimpleText(text.c_str(), text.size(), SkTextEncoding::kUTF8, x, y, font, paint);
    }
}

} // namespace MochiUI
