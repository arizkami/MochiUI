#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/gui/Components/TextNode.hpp>

namespace MochiUI {

class TooltipNode : public FlexNode {
public:
    TooltipNode(const std::string& text) {
        style.setPositionType(YGPositionTypeAbsolute);
        style.backgroundColor = SkColorSetRGB(40, 40, 40);
        style.setPadding(8, 4);
        style.borderRadius = 4;
        
        auto label = std::make_shared<TextNode>(text);
        label->color = SK_ColorWHITE;
        label->fontSize = Theme::FontSmall;
        addChild(label);
    }
};

} // namespace MochiUI
