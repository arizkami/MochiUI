#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <chrono>

namespace MochiUI {

class SkeletonNode : public FlexNode {
public:
    SkeletonNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        startTime = std::chrono::steady_clock::now();
    }

    Size measure(Size available) override {
        float w = IsUndefined(available.width) ? 100.0f : available.width;
        float h = IsUndefined(available.height) ? 20.0f : available.height;
        return { w, h };
    }

    void drawSelf(SkCanvas* canvas) override {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        
        float pulse = (sinf(elapsed * 0.005f) + 1.0f) / 2.0f; // 0 to 1
        SkColor base = Theme::Card;
        SkColor pulseColor = SkColorSetA(base, 100 + pulse * 100);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(pulseColor);
        
        if (style.borderRadius > 0) canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, paint);
        else canvas->drawRect(frame, paint);
    }

    bool needsRedraw() override { return true; }

private:
    std::chrono::steady_clock::time_point startTime;
};

} // namespace MochiUI
