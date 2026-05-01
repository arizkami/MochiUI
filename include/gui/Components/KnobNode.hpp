#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <functional>
#include <cmath>

namespace AureliaUI {

class KnobNode : public FlexNode {
public:
    KnobNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }
    float value = 0.5f;  // 0.0 to 1.0
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float defaultValue = 0.5f;
    std::function<void(float)> onValueChange;

    AUKColor knobBodyColor = AUKColor::RGB(45, 45, 48);
    AUKColor knobRingColor = AUKColor::RGB(60, 60, 65);
    AUKColor arcTrackColor = AUKColor::RGB(255, 255, 255, 40);
    AUKColor arcFillColor = Theme::Accent;
    AUKColor indicatorColor = Theme::Accent;
    AUKColor textColor = Theme::TextSecondary;

    float knobSize = 70.0f;
    float arcWidth = 4.0f;
    float startAngle = 135.0f;  // degrees
    float sweepAngle = 270.0f;  // degrees
    bool showValue = true;

    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
    bool hitTest(float x, float y) override;
    bool onMouseDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;
    void onMouseUp(float x, float y) override;
    bool onMouseWheel(float x, float y, float delta) override;
    bool onDoubleClick(float x, float y);

private:
    void updateValueFromPosition(float x, float y);
    void updateValueFromRotation(float x, float y);
    float getNormalizedValue() const;
    float getAngleForValue() const;
    bool isDragging = false;
    float lastMouseY = 0.0f;
    uint32_t lastClickTime = 0;
};

} // namespace AureliaUI
