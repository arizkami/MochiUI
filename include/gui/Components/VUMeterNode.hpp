#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFont.h>
#include <string>
#include <cmath>

namespace MochiUI {

class VUMeterNode : public FlexNode {
public:
    VUMeterNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }
    float value = 0.0f;  // -60dB to 0dB (or 0.0 to 1.0 normalized)
    float peakValue = 0.0f;
    bool vertical = true;
    bool showNumber = true;
    bool useDecibels = true;  // Show dB values instead of percentage
    
    // Colors
    SkColor backgroundColor = SkColorSetRGB(30, 30, 30);
    SkColor greenColor = SkColorSetRGB(0, 200, 100);
    SkColor yellowColor = SkColorSetRGB(255, 200, 0);
    SkColor redColor = SkColorSetRGB(255, 50, 50);
    SkColor peakColor = SK_ColorWHITE;
    SkColor textColor = Theme::TextSecondary;
    
    // Thresholds (in normalized 0-1 range)
    float yellowThreshold = 0.7f;   // -6dB
    float redThreshold = 0.9f;      // -3dB
    
    // Dimensions
    float meterWidth = 20.0f;
    float meterHeight = 200.0f;
    
    // Peak hold
    float peakHoldTime = 1.5f;  // seconds
    
    void setValue(float val);
    void setPeak(float val);
    void update(float deltaTime);  // Call this each frame to update peak hold
    
    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;

private:
    float valueToDB(float normalized) const;
    std::string formatValue(float val) const;
    SkColor getColorForValue(float val) const;
    
    float peakHoldTimer = 0.0f;
};

} // namespace MochiUI
