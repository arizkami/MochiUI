#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/TextNode.hpp>

namespace AureliaUI {

class TooltipNode : public FlexNode {
public:
    TooltipNode(const std::string& text) {
        style.setPositionType(YGPositionTypeAbsolute);
        style.backgroundColor = AUKColor::RGB(40, 40, 40);
        style.setPadding(8, 4);
        style.borderRadius = 4;

        auto label = std::make_shared<TextNode>(text);
        label->color = AUKColor::white();
        label->fontSize = Theme::FontSmall;
        addChild(label);
    }
};

} // namespace AureliaUI
