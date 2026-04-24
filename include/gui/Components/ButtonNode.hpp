#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <string>

namespace MochiUI {

class ButtonNode : public FlexNode {
public:
    ButtonNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }
    std::string label = "Button";
    SkColor textColor = Theme::TextPrimary;
    SkColor normalColor = Theme::Card;
    SkColor hoverColor = SkColorSetRGB(60, 60, 60);
    SkColor pressedColor = SkColorSetRGB(40, 40, 40);
    float borderRadius = 6.0f;
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
        SkColor bg = isPressed ? pressedColor : (isHovered ? hoverColor : normalColor);
        
        SkPaint bgPaint;
        bgPaint.setAntiAlias(true);
        bgPaint.setColor(bg);
        canvas->drawRoundRect(frame, borderRadius, borderRadius, bgPaint);

        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(textColor);
        
        SkRect bounds;
        FontManager::getInstance().measureText(label, fontSize, &bounds);
        
        float tx = frame.left() + (frame.width() - bounds.width()) / 2.0f;
        float ty = frame.top() + (frame.height() + bounds.height()) / 2.0f - 2.0f;
        
        FontManager::getInstance().drawText(canvas, label, tx, ty, fontSize, textPaint);
    }
};

} // namespace MochiUI
