#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/gui/Components/TextNode.hpp>
#include <string>
#include <vector>

namespace MochiUI {

class TabsNode : public FlexNode {
public:
    struct Tab {
        std::string title;
        FlexNode::Ptr content;
    };

    TabsNode() {
        header = FlexNode::Row();
        header->style.setHeight(Theme::ControlHeight + 8.0f);
        header->style.setGap(10);
        header->style.setPadding(4);
        
        contentArea = FlexNode::Create();
        contentArea->style.setFlex(1.0f);
        
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
            tabBtn->style.setPadding(12, 0);
            tabBtn->style.setHeightFull();
            tabBtn->enableHover = true;
            
            if (i == selectedIndex) {
                tabBtn->style.backgroundColor = SkColorSetA(Theme::Accent, 40);
            }

            auto label = std::make_shared<TextNode>(tabs[i].title);
            label->fontSize = Theme::FontNormal;
            label->color = (i == selectedIndex) ? Theme::Accent : Theme::TextPrimary;
            tabBtn->addChild(label);

            tabBtn->onClick = [this, i]() { selectTab(i); };
            header->addChild(tabBtn);
        }
    }

    std::vector<Tab> tabs;
    size_t selectedIndex = 0;
    FlexNode::Ptr header;
    FlexNode::Ptr contentArea;
};

} // namespace MochiUI
