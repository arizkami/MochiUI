#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/TextNode.hpp>

namespace AureliaUI {

class TimelineNode : public FlexNode {
public:
    TimelineNode() {
        style.setFlexDirection(YGFlexDirectionColumn);
        style.setGap(20);
    }

    void addEvent(const std::string& time, const std::string& title) {
        auto row = FlexNode::Row();
        row->style.setGap(15);

        auto timeNode = std::make_shared<TextNode>(time);
        timeNode->style.setWidth(80);
        timeNode->fontSize = Theme::FontSmall;
        timeNode->color = Theme::TextSecondary;
        row->addChild(timeNode);

        auto dot = FlexNode::Create();
        dot->style.setWidth(10);
        dot->style.setHeight(10);
        dot->style.borderRadius = 5;
        dot->style.backgroundColor = Theme::Accent;
        dot->style.setMargin(0, 5);
        row->addChild(dot);

        auto titleNode = std::make_shared<TextNode>(title);
        titleNode->fontSize = Theme::FontNormal;
        row->addChild(titleNode);

        addChild(row);
    }
};

} // namespace AureliaUI
