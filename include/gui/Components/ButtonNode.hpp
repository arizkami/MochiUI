#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>

namespace AureliaUI {

class ButtonNode : public FlexNode {
public:
    ButtonNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
    }
    std::string label    = "Button";
    AUKColor textColor   = AUKColor::black();
    AUKColor normalColor = AUKColor::RGB(235, 235, 235);  // browser button gray
    AUKColor hoverColor  = AUKColor::RGB(210, 210, 210);
    AUKColor pressedColor= AUKColor::RGB(188, 188, 188);
    AUKColor borderColor = AUKColor::RGB(168, 168, 168);  // 1px border like browser
    float borderWidth    = 1.0f;
    float borderRadius   = 3.0f;
    float fontSize       = 14.0f;
    bool  labelBold      = false;

    // When true, overrides the fields above with active Theme colors.
    bool useThemeColors  = false;

    Size measure(Size available) override {
        SkRect bounds = SkRect::MakeEmpty();
        if (labelBold) {
            SkFont font = FontManager::getInstance().createFont(FontManager::DEFAULT_FONT, fontSize, SkFontStyle::Bold());
            font.measureText(label.c_str(), label.size(), SkTextEncoding::kUTF8, &bounds);
        } else {
            FontManager::getInstance().measureText(label, fontSize, &bounds);
        }
        float w = bounds.width() + 16.0f;   // ~8px each side
        float h = bounds.height() + 8.0f;   // ~4px each side

        if (style.widthMode  == SizingMode::Fixed) w = style.width;
        if (style.heightMode == SizingMode::Fixed) h = style.height;

        return { w, h };
    }

    void draw(SkCanvas* canvas) override {
        AUKColor bg      = normalColor;
        AUKColor textCol = textColor;
        AUKColor border  = borderColor;
        float r = borderRadius;

        if (useThemeColors) {
            bg      = Theme::Card;
            textCol = Theme::TextPrimary;
            border  = AUKColor(Theme::Border);
            r       = Theme::BorderRadius;
        }

        if (isPressed) {
            bg = useThemeColors ? AUKColor(Theme::Accent).withAlpha(uint8_t(180)) : pressedColor;
        } else if (isHovered) {
            bg = useThemeColors ? AUKColor(Theme::Accent).withAlpha(uint8_t(100)) : hoverColor;
        }

        // background
        SkPaint bgPaint;
        bgPaint.setAntiAlias(true);
        bgPaint.setColor(bg);
        canvas->drawRoundRect(frame, r, r, bgPaint);

        // border
        if (borderWidth > 0.0f && border.a() > 0) {
            SkPaint borderPaint;
            borderPaint.setAntiAlias(true);
            borderPaint.setColor(border);
            borderPaint.setStyle(SkPaint::kStroke_Style);
            borderPaint.setStrokeWidth(borderWidth);
            SkRect inset = frame.makeInset(borderWidth * 0.5f, borderWidth * 0.5f);
            canvas->drawRoundRect(inset, r, r, borderPaint);
        }

        // label
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setColor(textCol);

        SkFontMetrics metrics{};
        float textWidth = 0;
        float tx = frame.left();
        float ty = frame.top();

        if (labelBold) {
            SkFont font = FontManager::getInstance().createFont(FontManager::DEFAULT_FONT, fontSize, SkFontStyle::Bold());
            font.getMetrics(&metrics);
            textWidth = font.measureText(label.c_str(), label.size(), SkTextEncoding::kUTF8);
            tx = frame.left() + (frame.width() - textWidth) / 2.0f;
            ty = frame.centerY() - (metrics.fAscent + metrics.fDescent) / 2.0f;
            canvas->drawSimpleText(label.c_str(), label.size(), SkTextEncoding::kUTF8, tx, ty, font, textPaint);
        } else {
            FontManager::getInstance().getFontMetrics(fontSize, &metrics);
            textWidth = FontManager::getInstance().measureText(label, fontSize);
            tx = frame.left() + (frame.width() - textWidth) / 2.0f;
            ty = frame.centerY() - (metrics.fAscent + metrics.fDescent) / 2.0f;
            FontManager::getInstance().drawText(canvas, label, tx, ty, fontSize, textPaint);
        }
    }
};

} // namespace AureliaUI
