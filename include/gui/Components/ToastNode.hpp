#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/gui/Components/TextNode.hpp>

namespace MochiUI {

class ToastNode : public FlexNode {
public:
    ToastNode(const std::string& message) {
        style.setPadding(16, 12);
        style.backgroundColor = Theme::Card;
        style.borderRadius = 8;
        style.setMargin(10);
        
        auto label = std::make_shared<TextNode>(message);
        label->fontSize = Theme::FontNormal;
        addChild(label);
    }
};

} // namespace MochiUI
