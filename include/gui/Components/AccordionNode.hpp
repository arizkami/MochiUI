#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/TextNode.hpp>
#include <string>
#include <vector>

namespace AureliaUI {

class AccordionItem : public FlexNode {
public:
    AccordionItem(const std::string& title, FlexNode::Ptr content) : content(content) {
        header = FlexNode::Row();
        header->style.setHeight(Theme::ControlHeight);
        header->style.setPadding(10, 0);
        header->style.setAlignItems(YGAlignStretch);
        header->enableHover = true;
        header->style.backgroundColor = Theme::Card;

        auto label = std::make_shared<TextNode>(title);
        label->fontSize = Theme::FontNormal;
        header->addChild(label);

        header->onClick = [this]() { setExpanded(!expanded); };

        addChild(header);

        contentWrapper = FlexNode::Create();
        contentWrapper->style.setPadding(10);
        contentWrapper->style.setHeightAuto();
        contentWrapper->addChild(content);

        setExpanded(false);
    }

    void setExpanded(bool e) {
        expanded = e;
        if (expanded) {
            if (children.size() == 1) addChild(contentWrapper);
            header->style.backgroundColor = SkColorSetA(Theme::Accent, 20);
        } else {
            if (children.size() == 2) removeChild(contentWrapper);
            header->style.backgroundColor = Theme::Card;
        }
    }

private:
    bool expanded = false;
    FlexNode::Ptr header;
    FlexNode::Ptr content;
    FlexNode::Ptr contentWrapper;
};

class AccordionNode : public FlexNode {
public:
    AccordionNode() {
        style.setGap(1); // Small line between items
    }

    void addItem(const std::string& title, FlexNode::Ptr content) {
        addChild(std::make_shared<AccordionItem>(title, content));
    }
};

} // namespace AureliaUI
