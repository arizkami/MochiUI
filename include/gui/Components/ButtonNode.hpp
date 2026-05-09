#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>

namespace SphereUI {

class ButtonNode : public FlexNode {
public:
    ButtonNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
    }
    std::string label    = "Button";
    bool labelBold       = false;
    bool useThemeColors  = true;
    SPHXColor normalColor = SPHXColor::transparent();
    SPHXColor hoverColor  = SPHXColor::transparent();
    SPHXColor textColor   = SPHXColor::transparent();
    SPHXColor borderColor = SPHXColor::transparent();
    float borderWidth    = -1.0f;
    float borderRadius   = -1.0f;
    float fontSize       = -1.0f;

    Size measure(Size available) override {
        float fs = nodeStyle.fontSize.value_or(fontSize >= 0.0f ? fontSize : Theme::FontNormal);
        SkRect bounds = SkRect::MakeEmpty();
        if (labelBold) {
            SkFont font = FontManager::getInstance().createFont(FontManager::DEFAULT_FONT, fs, SkFontStyle::Bold());
            font.measureText(label.c_str(), label.size(), SkTextEncoding::kUTF8, &bounds);
        } else {
            FontManager::getInstance().measureText(label, fs, &bounds);
        }
        float w = bounds.width() + 24.0f;
        float h = bounds.height() + 12.0f;

        if (style.widthMode  == SizingMode::Fixed) w = style.width;
        if (style.heightMode == SizingMode::Fixed) h = style.height;

        return { w, h };
    }

    void draw(SkCanvas* canvas) override {
        auto originalBg = nodeStyle.background;
        auto originalBorder = nodeStyle.border;
        auto originalFg = nodeStyle.foreground;
        auto originalFontSize = nodeStyle.fontSize;
        auto originalBorderRadius = nodeStyle.borderRadius;
        auto originalBorderWidth = nodeStyle.borderWidth;

        // Backfill from legacy fields so older call sites still feed the new style system.
        if (fontSize >= 0.0f && !nodeStyle.fontSize.has_value()) nodeStyle.fontSize = fontSize;
        if (textColor.a() > 0 && !nodeStyle.foreground.has_value()) nodeStyle.foreground = textColor;
        if (borderRadius >= 0.0f && !nodeStyle.borderRadius.has_value()) nodeStyle.borderRadius = borderRadius;
        if (borderWidth >= 0.0f && !nodeStyle.borderWidth.has_value()) nodeStyle.borderWidth = borderWidth;
        if (borderColor.a() > 0 && !nodeStyle.border.has_value()) nodeStyle.border = borderColor;

        if (useThemeColors) {
            if (isPressed) {
                nodeStyle.background = SPHXColor(Theme::Accent).withAlpha(uint8_t{180});
            } else if (isHovered) {
                nodeStyle.background = SPHXColor(Theme::Accent).withAlpha(uint8_t{100});
            } else {
                if (!nodeStyle.background.has_value()) nodeStyle.background = SPHXColor(Theme::Card);
                if (!nodeStyle.border.has_value())     nodeStyle.border = SPHXColor(Theme::Border);
            }
        } else {
            if (normalColor.a() > 0) nodeStyle.background = normalColor;
            if (isHovered && hoverColor.a() > 0) nodeStyle.background = hoverColor;
            if (borderColor.a() > 0) nodeStyle.border = borderColor;
        }
        if (!nodeStyle.borderRadius.has_value()) nodeStyle.borderRadius = Theme::BorderRadius;
        if (!nodeStyle.borderWidth.has_value())  nodeStyle.borderWidth = 1.0f;

        SPHXColor textCol = nodeStyle.foreground.value_or(SPHXColor(Theme::TextPrimary));
        float fs = nodeStyle.fontSize.value_or(fontSize >= 0.0f ? fontSize : Theme::FontNormal);

        drawSelf(canvas);

        nodeStyle.background = originalBg;
        nodeStyle.border = originalBorder;
        nodeStyle.foreground = originalFg;
        nodeStyle.fontSize = originalFontSize;
        nodeStyle.borderRadius = originalBorderRadius;
        nodeStyle.borderWidth = originalBorderWidth;

        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(textCol);

        SkFontMetrics metrics{};
        float textWidth = 0;
        float tx = frame.left();
        float ty = frame.top();

        if (labelBold) {
            SkFont font = FontManager::getInstance().createFont(FontManager::DEFAULT_FONT, fs, SkFontStyle::Bold());
            font.getMetrics(&metrics);
            textWidth = font.measureText(label.c_str(), label.size(), SkTextEncoding::kUTF8);
            tx = frame.left() + (frame.width() - textWidth) / 2.0f;
            ty = frame.centerY() - (metrics.fAscent + metrics.fDescent) / 2.0f;
            canvas->drawSimpleText(label.c_str(), label.size(), SkTextEncoding::kUTF8, tx, ty, font, textPaint);
        } else {
            FontManager::getInstance().getFontMetrics(fs, &metrics);
            textWidth = FontManager::getInstance().measureText(label, fs);
            tx = frame.left() + (frame.width() - textWidth) / 2.0f;
            ty = frame.centerY() - (metrics.fAscent + metrics.fDescent) / 2.0f;
            FontManager::getInstance().drawText(canvas, label, tx, ty, fs, textPaint);
        }

        drawChildren(canvas);
    }
};

} // namespace SphereUI
