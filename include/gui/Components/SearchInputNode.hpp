#pragma once
#include <gui/Components/TextInput.hpp>
#include <gui/Components/IconNode.hpp>

namespace AureliaUI {

class SearchInputNode : public FlexNode {
public:
    std::function<void(const std::string&)> onChanged;

    const std::string& getText() const { return input->text; }
    void setText(const std::string& t) {
        input->text = t;
        syncClearButton(!t.empty());
    }

    SearchInputNode() {
        style.setHeight(Theme::ControlHeight);
        style.setAlignItems(YGAlignCenter);
        style.setFlexDirection(YGFlexDirectionRow);
        style.backgroundColor = Theme::Card;
        style.borderRadius = Theme::BorderRadius;
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
        input->style.backgroundColor = AUKColor::transparent();
        input->style.setPadding(0);
        input->placeholder = "Search...";
        input->onChanged = [this](const std::string& text) {
            syncClearButton(!text.empty());
            if (onChanged) onChanged(text);
        };
        addChild(input);

        clearBtn = std::make_shared<IconNode>();
        clearBtn->setIcon("res://x.svg");
        clearBtn->style.setWidth(14);
        clearBtn->style.setHeight(14);
        clearBtn->color = Theme::TextSecondary;
        clearBtn->enableHover = true;
        clearBtn->onClick = [this]() {
            input->text.clear();
            syncClearButton(false);
            if (onChanged) onChanged("");
        };
        // clearBtn is added/removed dynamically via syncClearButton
    }

private:
    std::shared_ptr<TextInput> input;
    std::shared_ptr<IconNode>  clearBtn;
    bool clearBtnAdded = false;

    void syncClearButton(bool show) {
        if (show && !clearBtnAdded) {
            addChild(clearBtn);
            clearBtnAdded = true;
        } else if (!show && clearBtnAdded) {
            removeChild(clearBtn);
            clearBtnAdded = false;
        }
    }
};

} // namespace AureliaUI
