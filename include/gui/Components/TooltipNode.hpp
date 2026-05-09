#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/TextNode.hpp>

namespace SphereUI {

class TooltipNode : public FlexNode {
public:
    TooltipNode(const std::string& text) {
        style.setPositionType(YGPositionTypeAbsolute);
        style.backgroundColor = SPHXColor::RGB(40, 40, 40);
        style.setPadding(8, 4);
        style.borderRadius = 4;

        auto label = std::make_shared<TextNode>(text);
        label->color = SPHXColor::white();
        label->fontSize = Theme::FontSmall;
        addChild(label);
    }
};

} // namespace SphereUI
