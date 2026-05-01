#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <chrono>
#include <cmath>

namespace AureliaUI {

class ProgressBar : public FlexNode {
public:
    ProgressBar() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }

    float value = 0.0f; // 0.0 to 1.0 (determinate only)
    AUKColor fillColor = Theme::Accent;
    AUKColor backgroundColor = Theme::Sidebar;
    float borderRadius = 4.0f;
    bool showPercentage = false;

    // Indeterminate (“infinite”) bar: sliding segment; ignores `value` while enabled.
    bool indeterminate = false;
    // Width of the sliding segment as a fraction of the track (0..1).
    float indeterminateSegment = 0.35f;
    // Full back-and-forth cycle duration.
    std::chrono::milliseconds indeterminatePeriod{1600};

    void setIndeterminate(bool on) {
        if (indeterminate == on) {
            return;
        }
        indeterminate = on;
        markDirty();
    }

    Size measure(Size available) override {
        float w = (style.widthMode == SizingMode::Fixed) ? style.width
                                                         : (IsUndefined(available.width) ? 200.0f : available.width);
        float h = (style.heightMode == SizingMode::Fixed) ? style.height : 20.0f;
        return {w, h};
    }

    void draw(SkCanvas* canvas) override {
        FlexNode::draw(canvas);

        SkPaint bgPaint;
        bgPaint.setAntiAlias(true);
        bgPaint.setColor(backgroundColor);
        canvas->drawRoundRect(frame, borderRadius, borderRadius, bgPaint);

        SkPaint fillPaint;
        fillPaint.setAntiAlias(true);
        fillPaint.setColor(fillColor);

        if (indeterminate) {
            const auto now = std::chrono::steady_clock::now();
            const float ms = std::chrono::duration<float>(now.time_since_epoch()).count();
            const float periodMs = std::max(100.0f, static_cast<float>(indeterminatePeriod.count()));
            const float t = std::fmod(ms / periodMs, 1.0f);
            // Smooth ping-pong along the track (0 → 1 → 0).
            const float u = 0.5f + 0.5f * std::sin(t * 6.28318530718f);

            const float segFrac = std::clamp(indeterminateSegment, 0.08f, 0.95f);
            const float segW = frame.width() * segFrac;
            const float range = std::max(0.0f, frame.width() - segW);
            const float x0 = frame.fLeft + u * range;

            SkRect segRect = SkRect::MakeXYWH(x0, frame.fTop, segW, frame.height());
            const float segR = std::min(borderRadius, frame.height() * 0.5f);

            canvas->save();
            canvas->clipRRect(SkRRect::MakeRectXY(frame, borderRadius, borderRadius), SkClipOp::kIntersect, true);
            canvas->drawRoundRect(segRect, segR, segR, fillPaint);
            canvas->restore();
        } else if (value > 0.0f) {
            SkRect fillRect = frame;
            fillRect.fRight = frame.fLeft + (frame.width() * std::clamp(value, 0.0f, 1.0f));
            canvas->drawRoundRect(fillRect, borderRadius, borderRadius, fillPaint);
        }
    }

    bool needsRedraw() override {
        if (indeterminate) {
            return true;
        }
        return FlexNode::needsRedraw();
    }
};

} // namespace AureliaUI
