#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <functional>

namespace AureliaUI {

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

    AUKColor trackColor = Theme::Card;
    AUKColor fillColor = Theme::Accent;
    AUKColor thumbColor = AUKColor::white();

    float trackHeight = 4.0f;
    float thumbRadius = 8.0f;

    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;
    void onMouseUp(float x, float y) override;
    bool onMouseWheel(float x, float y, float delta) override;

private:
    void updateValueFromPosition(float x, float y);
    float getNormalizedValue() const;
    bool isDragging = false;
};

} // namespace AureliaUI
