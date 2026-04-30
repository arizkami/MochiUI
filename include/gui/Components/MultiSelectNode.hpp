#pragma once
#include <gui/Components/ComboBox.hpp>
#include <gui/Components/CheckboxNode.hpp>
#include <set>

namespace MochiUI {

class MultiSelectNode : public FlexNode {
public:
    MultiSelectNode() {
        style.setHeight(Theme::ControlHeight);
        style.backgroundColor = Theme::Card;
        style.setAlignItems(YGAlignCenter);
        style.setPadding(8, 0);
        
        label = std::make_shared<TextNode>("Select...");
        label->fontSize = Theme::FontNormal;
        addChild(label);
    }

    std::vector<std::string> items;
    std::set<int> selectedIndices;
    
private:
    std::shared_ptr<TextNode> label;
};

} // namespace MochiUI
