#pragma once
#include "Layout.hpp"
#include "Theme.hpp"
#include <include/core/SkFont.h>
#include <include/core/SkTypeface.h>
#include <include/core/SkFontMgr.h>
#include <string>

extern sk_sp<SkFontMgr> gFontMgr;

namespace MochiUI {

class TextNode : public FlexNode {
public:
    std::string text;
    SkColor color = SK_ColorBLACK;
    float fontSize = 16.0f;

    Size measure(Size available) override {
        if (text.empty()) return { 0, 0 };
        
        sk_sp<SkTypeface> typeface = gFontMgr->matchFamilyStyle("Segoe UI", SkFontStyle());
        if (!typeface) typeface = gFontMgr->legacyMakeTypeface(nullptr, SkFontStyle());
        
        SkFont font(typeface, fontSize);
        SkRect bounds;
        font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8, &bounds);
        
        float w = bounds.width() + 2 * style.padding;
        float h = bounds.height() + 2 * style.padding;
        
        if (style.widthMode == SizingMode::Fixed) w = style.width;
        if (style.heightMode == SizingMode::Fixed) h = style.height;
        
        return { w, h };
    }

    void draw(SkCanvas* canvas) override {
        // Draw hover effect
        if (isHovered) {
            SkPaint hoverPaint;
            hoverPaint.setAntiAlias(true);
            hoverPaint.setColor(Theme::HoverOverlay);
            canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, hoverPaint);
        }

        FlexNode::draw(canvas);
        if (!text.empty()) {
            sk_sp<SkTypeface> typeface = gFontMgr->matchFamilyStyle("Segoe UI", SkFontStyle());
            if (!typeface) typeface = gFontMgr->legacyMakeTypeface(nullptr, SkFontStyle());
            
            SkFont font(typeface, fontSize);
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
};

} // namespace MochiUI
