#pragma once
#include <gui/Layout.hpp>
#include <memory>

namespace MochiUI {

class FocusManager {
public:
    static FocusManager& getInstance() {
        static FocusManager instance;
        return instance;
    }

    void setFocusedNode(FlexNode::Ptr node) {
        if (focusedNode) focusedNode->isFocused = false;
        focusedNode = node;
        if (focusedNode) focusedNode->isFocused = true;
    }

    FlexNode::Ptr getFocusedNode() const { return focusedNode; }

    void clearFocus() {
        if (focusedNode) focusedNode->isFocused = false;
        focusedNode = nullptr;
    }

private:
    FocusManager() = default;
    FlexNode::Ptr focusedNode;
};

} // namespace MochiUI
