#pragma once
#include <gui/Layout.hpp>

namespace AureliaUI {

// Full-window layer for toasts: must not steal hits when empty or over transparent area.
class ToastLayerNode : public FlexNode {
public:
    bool hitTest(float x, float y) override {
        for (const auto& child : children) {
            if (child->hitTest(x, y)) return true;
        }
        return false;
    }
};

class OverlayNode : public FlexNode {
public:
    using Ptr = std::shared_ptr<OverlayNode>;
    OverlayNode() {
        style.setWidthPercent(100.0f);
        style.setHeightPercent(100.0f);

        toastLayer = std::make_shared<ToastLayerNode>();
        toastLayer->style.setWidthPercent(100.0f);
        toastLayer->style.setHeightPercent(100.0f);
        toastLayer->style.setPositionType(YGPositionTypeAbsolute);
        toastLayer->style.setPadding(20);
        toastLayer->style.setAlignItems(YGAlignFlexEnd); // Toast to the right/bottom

        // Children will be added to specific layers
        addChild(toastLayer);
    }

    void setMainContent(FlexNode::Ptr content) {
        if (mainContent) removeChild(mainContent);
        mainContent = content;
        if (mainContent) {
            mainContent->style.setWidthPercent(100.0f);
            mainContent->style.setHeightPercent(100.0f);
            // Insert at index 0 to be behind everything
            children.insert(children.begin(), mainContent);
            YGNodeInsertChild(getYGNode(), mainContent->getYGNode(), 0);
            mainContent->parent = this;
            mainContent->setWindowHost(windowHost);
        }
    }

    void addOverlay(FlexNode::Ptr overlay) {
        addChild(overlay);
    }

    void removeOverlay(FlexNode::Ptr overlay) {
        removeChild(overlay);
    }

    void addToast(FlexNode::Ptr toast) {
        toastLayer->addChild(toast);
    }

    bool onMouseDown(float x, float y) override {
        return FlexNode::onMouseDown(x, y);
    }

private:
    FlexNode::Ptr mainContent;
    FlexNode::Ptr toastLayer;
};

} // namespace AureliaUI
