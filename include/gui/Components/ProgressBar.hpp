#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>

namespace MochiUI {

class ProgressBar : public FlexNode {
public:
    ProgressBar() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }
    float value = 0.0f; // 0.0 to 1.0
    SkColor fillColor = Theme::Accent;
    SkColor backgroundColor = Theme::Sidebar;
    float borderRadius = 4.0f;
    bool showPercentage = false;

    Size measure(Size available) override {
        float w = (style.widthMode == SizingMode::Fixed) ? style.width : 
                  (IsUndefined(available.width) ? 200.0f : available.width);
        float h = (style.heightMode == SizingMode::Fixed) ? style.height : 20.0f;
        return { w, h };
    }

    void draw(SkCanvas* canvas) override {
        FlexNode::draw(canvas);

        SkPaint bgPaint;
        bgPaint.setAntiAlias(true);
        bgPaint.setColor(backgroundColor);
        canvas->drawRoundRect(frame, borderRadius, borderRadius, bgPaint);

        if (value > 0) {
            SkRect fillRect = frame;
            fillRect.fRight = frame.fLeft + (frame.width() * std::clamp(value, 0.0f, 1.0f));
            
            SkPaint fillPaint;
            fillPaint.setAntiAlias(true);
            fillPaint.setColor(fillColor);
            canvas->drawRoundRect(fillRect, borderRadius, borderRadius, fillPaint);
        }
    }
};

} // namespace MochiUI
