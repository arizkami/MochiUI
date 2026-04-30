#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <include/core/SkFontMetrics.h>
#include <string>

namespace MochiUI {

class ButtonNode : public FlexNode {
public:
    ButtonNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
    }
    std::string label = "Button";
    SkColor textColor = Theme::TextPrimary;
    SkColor normalColor = Theme::Card;
    
    // If true, will use Theme::* colors dynamically in draw()
    bool useThemeColors = true;
    
    SkColor hoverColor = SkColorSetRGB(60, 60, 60);
    SkColor pressedColor = SkColorSetRGB(40, 40, 40);
    float borderRadius = -1.0f; // -1 to use Theme::BorderRadius
    float fontSize = 14.0f;

    Size measure(Size available) override {
        SkRect bounds;
        FontManager::getInstance().measureText(label, fontSize, &bounds);
        float w = bounds.width() + 30.0f;
        float h = bounds.height() + 16.0f;
        
        if (style.widthMode == SizingMode::Fixed) w = style.width;
        if (style.heightMode == SizingMode::Fixed) h = style.height;
        
        return { w, h };
    }

    void draw(SkCanvas* canvas) override {
        SkColor bg = normalColor;
        SkColor textCol = textColor;
        
        if (useThemeColors) {
            bg = Theme::Card;
            textCol = Theme::TextPrimary;
        }

        if (isPressed) {
            bg = useThemeColors ? SkColorSetA(Theme::Accent, 180) : pressedColor;
        } else if (isHovered) {
            bg = useThemeColors ? SkColorSetA(Theme::Accent, 100) : hoverColor;
        }
        
        float r = (borderRadius < 0) ? Theme::BorderRadius : borderRadius;
        
        SkPaint bgPaint;
        bgPaint.setAntiAlias(true);
        bgPaint.setColor(bg);
        canvas->drawRoundRect(frame, r, r, bgPaint);

        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(textCol);
        
        SkFontMetrics metrics;
        FontManager::getInstance().getFontMetrics(fontSize, &metrics);

        float textWidth = FontManager::getInstance().measureText(label, fontSize);
        float tx = frame.left() + (frame.width() - textWidth) / 2.0f;
        float ty = frame.centerY() - (metrics.fAscent + metrics.fDescent) / 2.0f;
        
        FontManager::getInstance().drawText(canvas, label, tx, ty, fontSize, textPaint);
    }
};

} // namespace MochiUI
