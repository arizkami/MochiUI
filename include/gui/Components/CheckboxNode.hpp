#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>
#include <functional>

namespace AureliaUI {

class CheckboxNode : public FlexNode {
public:
    CheckboxNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
    }
    std::string label;
    bool checked = false;
    AUKColor checkboxColor = Theme::Accent;
    AUKColor labelColor = Theme::TextPrimary;
    float fontSize = 14.0f;
    float checkboxSize = 18.0f;
    float spacing = 8.0f;
    std::function<void(bool)> onChanged;

    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;
};

} // namespace AureliaUI
