#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/TextNode.hpp>
#include <gui/Components/IconNode.hpp>

namespace MochiUI {

class EmptyStateNode : public FlexNode {
public:
    EmptyStateNode(const std::string& title, const std::string& description, const std::string& icon = "res://archive.svg") {
        style.setFlex(1.0f);
        style.setAlignItems(YGAlignCenter);
        style.setGap(10);
        style.setPadding(40);

        auto iconNode = std::make_shared<IconNode>();
        iconNode->setIcon(icon);
        iconNode->style.setWidth(64);
        iconNode->style.setHeight(64);
        iconNode->color = SkColorSetA(Theme::TextSecondary, 128);
        addChild(iconNode);

        auto titleNode = std::make_shared<TextNode>(title);
        titleNode->fontSize = Theme::FontLarge;
        titleNode->color = Theme::TextPrimary;
        addChild(titleNode);

        auto descNode = std::make_shared<TextNode>(description);
        descNode->fontSize = Theme::FontNormal;
        descNode->color = Theme::TextSecondary;
        addChild(descNode);
    }
};

} // namespace MochiUI
