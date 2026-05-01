#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/TextNode.hpp>
#include <string>
#include <vector>

namespace AureliaUI {

class TabsNode : public FlexNode {
public:
    struct Tab {
        std::string title;
        FlexNode::Ptr content;
    };

    TabsNode() {
        header = FlexNode::Row();
        header->style.setHeight(Theme::ControlHeight + 12.0f);
        header->style.setGap(4);
        header->style.setPadding(4);
        header->style.backgroundColor = SkColorSetA(Theme::Card, 100);

        contentArea = FlexNode::Create();
        contentArea->style.setFlex(1.0f);
        contentArea->style.setWidthFull();
        // Keep tab content top-aligned; stretch would give the panel a tall hit-box and wasted space.
        contentArea->style.setAlignItems(YGAlignFlexStart);

        addChild(header);
        addChild(contentArea);
    }

    void addTab(const std::string& title, FlexNode::Ptr content) {
        tabs.push_back({title, content});
        if (tabs.size() == 1) selectTab(0);
        rebuildHeader();
    }

    void selectTab(size_t index) {
        if (index >= tabs.size()) return;
        selectedIndex = index;

        contentArea->removeAllChildren();
        contentArea->addChild(tabs[index].content);
        rebuildHeader();
    }

private:
    void rebuildHeader() {
        header->removeAllChildren();
        for (size_t i = 0; i < tabs.size(); ++i) {
            auto tabBtn = std::make_shared<FlexNode>();
            tabBtn->style.setPadding(20, 0); // Horizontal padding
            tabBtn->style.setHeightFull();
            tabBtn->style.flexDirection = FlexDirection::Column;
            tabBtn->style.setAlignItems(YGAlignCenter); // Horizontal center
            tabBtn->style.setJustifyContent(YGJustifyCenter); // Vertical center
            tabBtn->enableHover = true;
            tabBtn->style.borderRadius = 6.0f;

            bool isActive = (i == selectedIndex);

            if (isActive) {
                tabBtn->style.backgroundColor = SkColorSetA(Theme::Accent, 25);
            }

            auto label = std::make_shared<TextNode>(tabs[i].title);
            label->fontSize = Theme::FontNormal;
            label->color = isActive ? Theme::Accent : Theme::TextSecondary;
            label->textAlign = TextAlign::Center;
            tabBtn->addChild(label);

            // Active indicator bar at the bottom
            if (isActive) {
                auto indicator = FlexNode::Create();
                indicator->style.setPositionType(YGPositionTypeAbsolute);
                indicator->style.setPosition(YGEdgeBottom, 2);
                indicator->style.setPosition(YGEdgeLeft, 10);
                indicator->style.setPosition(YGEdgeRight, 10);
                indicator->style.setHeight(3);
                indicator->style.backgroundColor = Theme::Accent;
                indicator->style.borderRadius = 2;
                tabBtn->addChild(indicator);
            }

            tabBtn->onClick = [this, i]() { selectTab(i); };
            header->addChild(tabBtn);
        }
    }

    std::vector<Tab> tabs;
    size_t selectedIndex = 0;
    FlexNode::Ptr header;
    FlexNode::Ptr contentArea;
};

} // namespace AureliaUI
