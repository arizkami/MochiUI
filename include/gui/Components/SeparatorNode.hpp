#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>

namespace SphereUI {

class SeparatorNode : public FlexNode {
public:
    enum class Orientation { Horizontal, Vertical };

    SeparatorNode(Orientation orientation = Orientation::Horizontal)
        : orientation(orientation) {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }

    Orientation orientation;
    SPHXColor color = SPHXColor(Theme::TextSecondary).withAlpha(uint8_t(50));
    float thickness = -1.0f; // -1 to use Theme::BorderWidth
    float margin = 8.0f;

    Size measure(Size available) override {
        float t = (thickness < 0) ? Theme::BorderWidth : thickness;
        float w = IsUndefined(available.width) ? 1.0f : available.width;
        float h = IsUndefined(available.height) ? 1.0f : available.height;

        if (orientation == Orientation::Horizontal) {
            return { w, t + margin * 2 };
        } else {
            return { t + margin * 2, h };
        }
    }

    void draw(SkCanvas* canvas) override {
        if (!canvas) return;

        drawSelf(canvas);

        float t = (thickness < 0) ? Theme::BorderWidth : thickness;
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(color);
        paint.setStrokeWidth(t);

        if (orientation == Orientation::Horizontal) {
            float y = frame.centerY();
            canvas->drawLine(frame.left() + margin, y, frame.right() - margin, y, paint);
        } else {
            float x = frame.centerX();
            canvas->drawLine(x, frame.top() + margin, x, frame.bottom() - margin, paint);
        }
    }
};

} // namespace SphereUI
