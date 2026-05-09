#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>

namespace SphereUI {

class BadgeNode : public FlexNode {
public:
    BadgeNode(std::string text = "") : text(text) {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        style.setPadding(4);
    }

    std::string text;
    SPHXColor color = SPHXColor::transparent();
    SPHXColor textColor = SPHXColor::transparent();
    float fontSize = -1.0f;

    Size measure(Size available) override {
        float fs = nodeStyle.fontSize.value_or(fontSize >= 0.0f ? fontSize : 10.0f);
        SkRect bounds;
        FontManager::getInstance().measureText(text, fs, &bounds);
        float w = std::max(bounds.width() + 10.0f, 16.0f);
        float h = std::max(bounds.height() + 4.0f, 16.0f);
        return { w, h };
    }

    void draw(SkCanvas* canvas) override {
        if (!canvas) return;

        auto originalBg = nodeStyle.background;
        auto originalFg = nodeStyle.foreground;
        auto originalFontSize = nodeStyle.fontSize;
        auto originalBorderRadius = nodeStyle.borderRadius;

        if (color.a() > 0 && !nodeStyle.background.has_value()) nodeStyle.background = color;
        if (textColor.a() > 0 && !nodeStyle.foreground.has_value()) nodeStyle.foreground = textColor;
        if (fontSize >= 0.0f && !nodeStyle.fontSize.has_value()) nodeStyle.fontSize = fontSize;

        if (!nodeStyle.background.has_value()) nodeStyle.background = SPHXColor(Theme::Accent);
        if (!nodeStyle.foreground.has_value()) nodeStyle.foreground = SPHXColor::white();
        
        float radius = frame.height() / 2.0f;
        if (!nodeStyle.borderRadius.has_value()) nodeStyle.borderRadius = radius;

        drawSelf(canvas);

        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(*nodeStyle.foreground);

        float fs = nodeStyle.fontSize.value_or(fontSize >= 0.0f ? fontSize : 10.0f);
        SkRect bounds;
        FontManager::getInstance().measureText(text, fs, &bounds);

        float textX = frame.centerX() - bounds.width() / 2;
        float textY = frame.centerY() - bounds.centerY();

        FontManager::getInstance().drawText(canvas, text, textX, textY, fs, textPaint);

        nodeStyle.background = originalBg;
        nodeStyle.foreground = originalFg;
        nodeStyle.fontSize = originalFontSize;
        nodeStyle.borderRadius = originalBorderRadius;
    }
};

} // namespace SphereUI
