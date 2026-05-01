#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>
#include <functional>

namespace AureliaUI {

class RadioButtonNode : public FlexNode {
public:
    RadioButtonNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
    }

    std::string label;
    bool selected = false;
    AUKColor radioColor = Theme::Accent;
    AUKColor labelColor = Theme::TextPrimary;

    float fontSize = 14.0f;
    float radioSize = 18.0f;
    float spacing = 8.0f;

    std::function<void(bool)> onSelected;

    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;
};

} // namespace AureliaUI
