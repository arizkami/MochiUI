#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <functional>
#include <optional>

namespace SphereUI {

struct SliderNodeStyle {
    std::optional<SPHXColor> trackColor;
    std::optional<SPHXColor> fillColor;
    std::optional<SPHXColor> thumbColor;
    std::optional<SPHXColor> thumbBorderColor;
    std::optional<SPHXColor> shadowColor;
    std::optional<float> trackHeight;
    std::optional<float> thumbRadius;
    std::optional<float> thumbBorderWidth;
};

class SliderNode : public FlexNode {
public:
    SliderNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }
    float value = 0.5f;  // 0.0 to 1.0
    float minValue = 0.0f;
    float maxValue = 1.0f;
    bool vertical = false;
    std::function<void(float)> onValueChange;

    SPHXColor trackColor = SPHXColor(Theme::Border).withAlpha(uint8_t{180});
    SPHXColor fillColor = SPHXColor(Theme::Accent);
    SPHXColor thumbColor = SPHXColor(Theme::Card).lighter(0.15f);
    float trackHeight = 6.0f;
    float thumbRadius = 10.0f;

    SliderNodeStyle visualStyle;

    void setStyleOverrides(const SliderNodeStyle& styleOverrides) { visualStyle = styleOverrides; }
    void clearStyleOverrides() { visualStyle = {}; }

    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;
    void onMouseUp(float x, float y) override;
    bool onMouseWheel(float x, float y, float delta) override;

private:
    SliderNodeStyle resolveStyle() const;
    void updateValueFromPosition(float x, float y);
    float getNormalizedValue() const;
    bool isDragging = false;
};

} // namespace SphereUI
