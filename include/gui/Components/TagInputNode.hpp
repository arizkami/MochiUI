#pragma once
#include <gui/Components/TextInput.hpp>
#include <gui/Components/BadgeNode.hpp>

namespace MochiUI {

class TagInputNode : public FlexNode {
public:
    TagInputNode() {
        style.setFlexDirection(YGFlexDirectionRow);
        style.setAlignItems(YGAlignCenter);
        style.flexWrap = YGWrapWrap;
        style.setGap(5);
        style.setPadding(5);
        style.backgroundColor = Theme::Card;
        style.setMinWidth(200);
        style.setHeightAuto();

        input = std::make_shared<TextInput>();
        input->style.setWidth(100);
        input->style.backgroundColor = SK_ColorTRANSPARENT;
        input->placeholder = "Add tag...";
        
        input->onEnter = [this]() {
            if (!input->text.empty()) {
                addTag(input->text);
                input->text = "";
            }
        };

        addChild(input);
    }

    void addTag(const std::string& text) {
        auto tag = std::make_shared<BadgeNode>(text);
        tag->style.setMargin(2);
        // Add before input
        children.insert(children.begin() + (children.size() - 1), tag);
        YGNodeInsertChild(getYGNode(), tag->getYGNode(), (uint32_t)(children.size() - 2));
        tag->parent = this;
    }

private:
    std::shared_ptr<TextInput> input;
};

} // namespace MochiUI
