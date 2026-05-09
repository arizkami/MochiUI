#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <functional>
#include <cmath>
#include <optional>

namespace SphereUI {

struct KnobNodeStyle {
    std::optional<SPHXColor> knobBodyColor;
    std::optional<SPHXColor> knobRingColor;
    std::optional<SPHXColor> arcTrackColor;
    std::optional<SPHXColor> arcFillColor;
    std::optional<SPHXColor> indicatorColor;
    std::optional<SPHXColor> shadowColor;
    std::optional<SPHXColor> glowColor;
    std::optional<float> knobSize;
    std::optional<float> arcWidth;
};

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

    SPHXColor knobBodyColor = SPHXColor(Theme::Card).darker(0.15f);
    SPHXColor knobRingColor = SPHXColor(Theme::Border).withAlpha(uint8_t{220});
    SPHXColor arcTrackColor = SPHXColor(Theme::Border).withAlpha(uint8_t{140});
    SPHXColor arcFillColor = SPHXColor(Theme::Accent);
    SPHXColor indicatorColor = SPHXColor(Theme::Accent).lighter(0.1f);
    float knobSize = 70.0f;
    float arcWidth = 4.0f;

    KnobNodeStyle visualStyle;
    float startAngle = 135.0f;  // degrees
    float sweepAngle = 270.0f;  // degrees
    bool showValue = true;
    /** When true, paint the normalized value as a percentage; otherwise use `valueDecimals` on the raw `value`. */
    bool showValueAsPercent = true;
    int valueDecimals = 0;
    SPHXColor valueLabelColor = SPHXColor(Theme::TextSecondary);

    void setStyleOverrides(const KnobNodeStyle& styleOverrides) { visualStyle = styleOverrides; }
    void clearStyleOverrides() { visualStyle = {}; }

    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
    bool hitTest(float x, float y) override;
    bool onMouseDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;
    void onMouseUp(float x, float y) override;
    bool onMouseWheel(float x, float y, float delta) override;
    bool onDoubleClick(float x, float y);

private:
    KnobNodeStyle resolveStyle() const;
    void updateValueFromPosition(float x, float y);
    void updateValueFromRotation(float x, float y);
    float getNormalizedValue() const;
    float getAngleForValue() const;
    bool isDragging = false;
    float lastMouseY = 0.0f;
    uint32_t lastClickTime = 0;
};

} // namespace SphereUI
