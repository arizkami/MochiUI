#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <functional>

namespace MochiUI {

class SliderNode : public FlexNode {
public:
    float value = 0.5f;  // 0.0 to 1.0
    float minValue = 0.0f;
    float maxValue = 1.0f;
    bool vertical = false;
    std::function<void(float)> onValueChange;
    
    SkColor trackColor = Theme::Card;
    SkColor fillColor = Theme::Accent;
    SkColor thumbColor = SK_ColorWHITE;
    
    float trackHeight = 4.0f;
    float thumbRadius = 8.0f;

    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;
    void onMouseUp(float x, float y) override;

private:
    void updateValueFromPosition(float x, float y);
    float getNormalizedValue() const;
    bool isDragging = false;
};

} // namespace MochiUI
