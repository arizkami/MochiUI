#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/TextNode.hpp>
#include <gui/Components/IconNode.hpp>

namespace MochiUI {

class TreeItem : public FlexNode {
public:
    TreeItem(const std::string& label, bool hasChildren = false) {
        style.setFlexDirection(YGFlexDirectionColumn);
        
        header = FlexNode::Row();
        header->style.setHeight(Theme::ControlHeight);
        header->style.setAlignItems(YGAlignCenter);
        header->style.setGap(5);
        header->enableHover = true;

        toggleIcon = std::make_shared<IconNode>();
        toggleIcon->setIcon("res://chevron-right.svg");
        toggleIcon->style.setWidth(12);
        toggleIcon->style.setHeight(12);
        toggleIcon->color = hasChildren ? Theme::TextSecondary : SK_ColorTRANSPARENT;
        header->addChild(toggleIcon);

        auto text = std::make_shared<TextNode>(label);
        text->fontSize = Theme::FontNormal;
        header->addChild(text);

        addChild(header);

        childContainer = FlexNode::Column();
        childContainer->style.setPadding(20, 0, 0, 0); // Indent
        
        header->onClick = [this, hasChildren]() {
            if (hasChildren) setExpanded(!expanded);
        };
    }

    void setExpanded(bool e) {
        expanded = e;
        if (expanded) {
            addChild(childContainer);
            toggleIcon->setIcon("res://chevron-down.svg");
        } else {
            removeChild(childContainer);
            toggleIcon->setIcon("res://chevron-right.svg");
        }
    }

    void addSubItem(std::shared_ptr<TreeItem> item) {
        childContainer->addChild(item);
        toggleIcon->color = Theme::TextSecondary;
    }

private:
    bool expanded = false;
    FlexNode::Ptr header;
    FlexNode::Ptr childContainer;
    std::shared_ptr<IconNode> toggleIcon;
};

class TreeView : public FlexNode {
public:
    TreeView() {
        style.setFlexDirection(YGFlexDirectionColumn);
    }
};

} // namespace MochiUI
