#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <gui/Components/ButtonNode.hpp>
#include <gui/Components/TextInput.hpp>

namespace AureliaUI {

class FilePickerNode : public FlexNode {
public:
    FilePickerNode() {
        style.setFlexDirection(YGFlexDirectionRow);
        style.setGap(8);
        style.setAlignItems(YGAlignCenter);

        pathInput = std::make_shared<TextInput>();
        pathInput->style.setFlex(1.0f);
        pathInput->placeholder = "No file selected";
        addChild(pathInput);

        browseBtn = std::make_shared<ButtonNode>();
        browseBtn->label = "Browse...";
        browseBtn->style.setWidth(80);
        addChild(browseBtn);
    }

private:
    std::shared_ptr<TextInput> pathInput;
    std::shared_ptr<ButtonNode> browseBtn;
};

} // namespace AureliaUI
