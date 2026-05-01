#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>

namespace AureliaUI {

class BadgeNode : public FlexNode {
public:
    BadgeNode(std::string text = "") : text(text) {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        style.setPadding(4);
    }

    std::string text;
    float fontSize = 10.0f;
    AUKColor color = Theme::Accent;
    AUKColor textColor = AUKColor::white();

    Size measure(Size available) override {
        SkRect bounds;
        FontManager::getInstance().measureText(text, fontSize, &bounds);
        float w = std::max(bounds.width() + 10.0f, 16.0f);
        float h = std::max(bounds.height() + 4.0f, 16.0f);
        return { w, h };
    }

    void draw(SkCanvas* canvas) override {
        if (!canvas) return;

        drawSelf(canvas);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(color);

        float radius = frame.height() / 2.0f;
        canvas->drawRoundRect(frame, radius, radius, paint);

        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(textColor);

        SkRect bounds;
        FontManager::getInstance().measureText(text, fontSize, &bounds);

        float textX = frame.centerX() - bounds.width() / 2;
        float textY = frame.centerY() - bounds.centerY();

        FontManager::getInstance().drawText(canvas, text, textX, textY, fontSize, textPaint);
    }
};

} // namespace AureliaUI
