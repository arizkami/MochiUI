#include <include/gui/Components/TextNode.hpp>
#include <include/core/SkFontMetrics.h>

namespace MochiUI {

Size TextNode::measure(Size available) {
    if (text.empty()) return { 0, 0 };
    SkRect bounds;
    FontManager::getInstance().measureText(text, fontSize, &bounds, fontFamily);
    // Use ceil to avoid sub-pixel cropping
    return { std::ceil(bounds.width()), std::ceil(bounds.height()) };
}

void TextNode::draw(SkCanvas* canvas) {
    drawSelf(canvas);

    if (!text.empty()) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(color);
        
        SkFontMetrics metrics;
        FontManager::getInstance().getFontMetrics(fontSize, &metrics, fontFamily);
        
        float x = frame.left() + style.padding;
        
        // Center based on full font bounds (ascent + descent) for better balance
        float y = frame.centerY() - (metrics.fAscent + metrics.fDescent) / 2.0f;
        
        // If we have fixed size or flex, we center horizontally. If it's hug, padding handles it.
        if (style.widthMode != SizingMode::Hug) {
            float textWidth = FontManager::getInstance().measureText(text, fontSize, nullptr, fontFamily);
            x = frame.left() + (frame.width() - textWidth) / 2.0f;
        }
        
        FontManager::getInstance().drawText(canvas, text, x, y, fontSize, paint, fontFamily);
    }

    drawChildren(canvas);
}
} // namespace MochiUI
