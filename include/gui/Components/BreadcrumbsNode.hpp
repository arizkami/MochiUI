#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/TextNode.hpp>
#include <gui/Components/LinkNode.hpp>
#include <vector>
#include <string>

namespace AureliaUI {

class BreadcrumbsNode : public FlexNode {
public:
    BreadcrumbsNode() {
        style.setFlexDirection(YGFlexDirectionRow);
        style.setAlignItems(YGAlignStretch);
        style.setGap(8);
    }

    void setPath(const std::vector<std::string>& path) {
        removeAllChildren();
        for (size_t i = 0; i < path.size(); ++i) {
            if (i > 0) {
                auto sep = std::make_shared<TextNode>("/");
                sep->color = Theme::TextSecondary;
                sep->fontSize = Theme::FontSmall;
                addChild(sep);
            }

            if (i == path.size() - 1) {
                auto label = std::make_shared<TextNode>(path[i]);
                label->fontSize = Theme::FontNormal;
                label->color = Theme::TextPrimary;
                addChild(label);
            } else {
                auto link = std::make_shared<LinkNode>(path[i]);
                link->fontSize = Theme::FontNormal;
                addChild(link);
            }
        }
    }
};

} // namespace AureliaUI
