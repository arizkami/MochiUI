#pragma once
#include <include/gui/Components/TextInput.hpp>
#include <include/gui/Components/IconNode.hpp>

namespace MochiUI {

class SearchInputNode : public FlexNode {
public:
    SearchInputNode() {
        style.setHeight(Theme::ControlHeight);
        style.setAlignItems(YGAlignCenter);
        style.setFlexDirection(YGFlexDirectionRow);
        style.backgroundColor = Theme::Card;
        style.setPadding(8, 0);
        style.setGap(8);

        auto searchIcon = std::make_shared<IconNode>();
        searchIcon->setIcon("res://search.svg"); 
        searchIcon->style.setWidth(16);
        searchIcon->style.setHeight(16);
        searchIcon->color = Theme::TextSecondary;
        addChild(searchIcon);

        input = std::make_shared<TextInput>();
        input->style.setFlex(1.0f);
        input->style.backgroundColor = SK_ColorTRANSPARENT;
        input->style.setPadding(0);
        input->placeholder = "Search...";
        addChild(input);

        clearBtn = std::make_shared<IconNode>();
        clearBtn->setIcon("res://x.svg"); 
        clearBtn->style.setWidth(14);
        clearBtn->style.setHeight(14);
        clearBtn->color = Theme::TextSecondary;
        clearBtn->enableHover = true;
        clearBtn->onClick = [this]() { input->text = ""; };
        // addChild(clearBtn); // only show when text not empty?
    }

private:
    std::shared_ptr<TextInput> input;
    std::shared_ptr<IconNode> clearBtn;
};

} // namespace MochiUI
