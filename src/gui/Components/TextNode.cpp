#include <gui/Components/TextNode.hpp>

namespace AureliaUI {

Size TextNode::measure(Size available) {
    if (text.empty()) return { 0, 0 };

    float width = 0;
    float height = 0;
    if (fontBold) {
        SkFont font = FontManager::getInstance().createFont(fontFamily, fontSize, SkFontStyle::Bold());
        width = font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8);
        SkFontMetrics metrics;
        font.getMetrics(&metrics);
        height = std::abs(metrics.fAscent) + std::abs(metrics.fDescent);
    } else {
        width = FontManager::getInstance().measureText(text, fontSize, nullptr, fontFamily);
        SkFontMetrics metrics;
        FontManager::getInstance().getFontMetrics(fontSize, &metrics, fontFamily);
        height = std::abs(metrics.fAscent) + std::abs(metrics.fDescent);
    }

    width += style.paddingLeft + style.paddingRight;
    height += style.paddingTop + style.paddingBottom;

    return { std::ceil(width), std::ceil(height) };
}

void TextNode::draw(SkCanvas* canvas) {
    drawSelf(canvas);

    if (!text.empty()) {
        canvas->save();
        canvas->clipRect(frame);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(color);

        SkFontMetrics metrics{};
        float textWidth = 0;
        float x = frame.left() + getLayoutPadding(YGEdgeLeft);

        if (fontBold) {
            SkFont font = FontManager::getInstance().createFont(fontFamily, fontSize, SkFontStyle::Bold());
            font.getMetrics(&metrics);
            textWidth = font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8);
            if (textAlign == TextAlign::Center) {
                x = frame.left() + (frame.width() - textWidth) / 2.0f;
            } else if (textAlign == TextAlign::Right) {
                x = frame.right() - getLayoutPadding(YGEdgeRight) - textWidth;
            }
            float y = frame.centerY() - (metrics.fAscent + metrics.fDescent) / 2.0f;
            x = std::round(x);
            y = std::round(y);
            canvas->drawSimpleText(text.c_str(), text.size(), SkTextEncoding::kUTF8, x, y, font, paint);
        } else {
            FontManager::getInstance().getFontMetrics(fontSize, &metrics, fontFamily);
            textWidth = FontManager::getInstance().measureText(text, fontSize, nullptr, fontFamily);
            if (textAlign == TextAlign::Center) {
                x = frame.left() + (frame.width() - textWidth) / 2.0f;
            } else if (textAlign == TextAlign::Right) {
                x = frame.right() - getLayoutPadding(YGEdgeRight) - textWidth;
            }
            float y = frame.centerY() - (metrics.fAscent + metrics.fDescent) / 2.0f;
            x = std::round(x);
            y = std::round(y);
            FontManager::getInstance().drawText(canvas, text, x, y, fontSize, paint, fontFamily);
        }

        canvas->restore();
    }

    drawChildren(canvas);
}
} // namespace AureliaUI
