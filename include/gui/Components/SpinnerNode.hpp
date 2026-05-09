#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <chrono>

namespace SphereUI {

class SpinnerNode : public FlexNode {
public:
    SpinnerNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        startTime = std::chrono::high_resolution_clock::now();
    }

    SPHXColor color = Theme::Accent;
    float size = 24.0f;
    float thickness = 3.0f;

    Size measure(Size available) override {
        return { size, size };
    }

    void draw(SkCanvas* canvas) override {
        if (!canvas) return;

        drawSelf(canvas);

        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
        // 360 degrees per second = 360 / 1,000,000 degrees per microsecond
        float angle = (duration % 1000000) * 0.00036f;

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(thickness);
        paint.setColor(color.withAlpha(uint8_t(50)));

        float radius = (size - thickness) / 2.0f;
        float centerX = frame.centerX();
        float centerY = frame.centerY();

        canvas->drawCircle(centerX, centerY, radius, paint);

        paint.setColor(color);
        paint.setStrokeCap(SkPaint::kRound_Cap);

        SkRect rect = SkRect::MakeXYWH(centerX - radius, centerY - radius, radius * 2, radius * 2);
        // Add a bit of length animation too for a more "modern" feel
        float sweep = 60.0f + 30.0f * sinf(duration * 0.000005f);
        canvas->drawArc(rect, angle, sweep, false, paint);
    }

    bool needsRedraw() override {
        return true; // Spinners always need redraw for animation
    }

private:
    std::chrono::high_resolution_clock::time_point startTime;
};

} // namespace SphereUI
