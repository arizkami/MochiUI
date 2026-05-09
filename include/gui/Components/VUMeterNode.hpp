#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>
#include <cmath>

namespace SphereUI {

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
    SPHXColor backgroundColor = SPHXColor::RGB(30, 30, 30);
    SPHXColor greenColor = SPHXColor::RGB(0, 200, 100);
    SPHXColor yellowColor = SPHXColor::RGB(255, 200, 0);
    SPHXColor redColor = SPHXColor::RGB(255, 50, 50);
    SPHXColor peakColor = SPHXColor::white();
    SPHXColor textColor = Theme::TextSecondary;

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
    SPHXColor getColorForValue(float val) const;

    float peakHoldTimer = 0.0f;
};

} // namespace SphereUI
