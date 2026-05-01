#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>
#include <functional>

namespace AureliaUI {

class SwitchNode : public FlexNode {
public:
    SwitchNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
    }

    std::string label;
    bool isOn = false;
    AUKColor activeColor = Theme::Accent;
    AUKColor inactiveColor = AUKColor::RGB(150, 150, 150);
    AUKColor thumbColor = AUKColor::white();
    AUKColor labelColor = Theme::TextPrimary;

    float fontSize = 14.0f;
    float switchWidth = 40.0f;
    float switchHeight = 22.0f;
    float spacing = 8.0f;

    std::function<void(bool)> onChanged;

    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;
};

} // namespace AureliaUI
