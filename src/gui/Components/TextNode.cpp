#include <include/gui/Components/TextNode.hpp>
#include <include/core/SkFontMetrics.h>

namespace MochiUI {

Size TextNode::measure(Size available) {
    if (text.empty()) return { 0, 0 };
    
    float width = FontManager::getInstance().measureText(text, fontSize, nullptr, fontFamily);
    
    SkFontMetrics metrics;
    FontManager::getInstance().getFontMetrics(fontSize, &metrics, fontFamily);
    float height = std::abs(metrics.fAscent) + std::abs(metrics.fDescent);
    
    return { std::ceil(width), std::ceil(height) };
}

void TextNode::draw(SkCanvas* canvas) {
    drawSelf(canvas);

    if (!text.empty()) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(color);
        
        SkFontMetrics metrics;
        FontManager::getInstance().getFontMetrics(fontSize, &metrics, fontFamily);
        
        float textWidth = FontManager::getInstance().measureText(text, fontSize, nullptr, fontFamily);
        float x = frame.left() + style.padding;
        
        if (textAlign == TextAlign::Center) {
            x = frame.left() + (frame.width() - textWidth) / 2.0f;
        } else if (textAlign == TextAlign::Right) {
            x = frame.right() - style.padding - textWidth;
        }
        
        // Center based on font metrics for consistent baseline across different characters
        float y = frame.centerY() - (metrics.fAscent + metrics.fDescent) / 2.0f;
        
        // Round to nearest pixel to avoid sub-pixel blurring
        x = std::round(x);
        y = std::round(y);
        
        FontManager::getInstance().drawText(canvas, text, x, y, fontSize, paint, fontFamily);
    }

    drawChildren(canvas);
}
} // namespace MochiUI
