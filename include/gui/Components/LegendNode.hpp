#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/TextNode.hpp>

namespace AureliaUI {

class LegendNode : public FlexNode {
public:
    LegendNode() {
        style.setFlexDirection(YGFlexDirectionRow);
        style.setGap(15);
        style.flexWrap = YGWrapWrap;
    }

    void addEntry(const std::string& label, SkColor color) {
        auto entry = FlexNode::Row();
        entry->style.setAlignItems(YGAlignCenter);
        entry->style.setGap(5);

        auto box = FlexNode::Create();
        box->style.setWidth(12);
        box->style.setHeight(12);
        box->style.backgroundColor = color;
        entry->addChild(box);

        auto text = std::make_shared<TextNode>(label);
        text->fontSize = Theme::FontSmall;
        entry->addChild(text);

        addChild(entry);
    }
};

} // namespace AureliaUI
