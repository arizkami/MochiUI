#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/TextNode.hpp>

namespace SphereUI {

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

} // namespace SphereUI
