#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <string>
#include <functional>

namespace MochiUI {

class SwitchNode : public FlexNode {
public:
    SwitchNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
    }
    
    std::string label;
    bool isOn = false;
    SkColor activeColor = Theme::Accent;
    SkColor inactiveColor = SkColorSetRGB(150, 150, 150);
    SkColor thumbColor = SK_ColorWHITE;
    SkColor labelColor = Theme::TextPrimary;
    
    float fontSize = 14.0f;
    float switchWidth = 40.0f;
    float switchHeight = 22.0f;
    float spacing = 8.0f;
    
    std::function<void(bool)> onChanged;
    
    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;
};

} // namespace MochiUI
