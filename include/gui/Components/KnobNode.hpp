#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPath.h>
#include <functional>
#include <cmath>

namespace MochiUI {

class KnobNode : public FlexNode {
public:
    float value = 0.5f;  // 0.0 to 1.0
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float defaultValue = 0.5f;
    std::function<void(float)> onValueChange;
    
    SkColor knobBodyColor = SkColorSetRGB(45, 45, 48);
    SkColor knobRingColor = SkColorSetRGB(60, 60, 65);
    SkColor arcTrackColor = SkColorSetARGB(40, 255, 255, 255);
    SkColor arcFillColor = Theme::Accent;
    SkColor indicatorColor = Theme::Accent;
    SkColor textColor = Theme::TextSecondary;
    
    float knobSize = 70.0f;
    float arcWidth = 4.0f;
    float startAngle = 135.0f;  // degrees
    float sweepAngle = 270.0f;  // degrees
    bool showValue = true;

    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;
    void onMouseUp(float x, float y) override;
    bool onDoubleClick(float x, float y);

private:
    void updateValueFromPosition(float x, float y);
    float getNormalizedValue() const;
    float getAngleForValue() const;
    bool isDragging = false;
    float lastMouseY = 0.0f;
    uint32_t lastClickTime = 0;
};

} // namespace MochiUI
