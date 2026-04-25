#pragma once
#include <include/gui/Layout.hpp>

namespace MochiUI {

class OverlayNode : public FlexNode {
public:
    using Ptr = std::shared_ptr<OverlayNode>;
    OverlayNode() {
        style.setWidthPercent(100.0f);
        style.setHeightPercent(100.0f);
        style.setPositionType(YGPositionTypeStatic);
    }

    void setMainContent(FlexNode::Ptr content) {
        if (mainContent) removeChild(mainContent);
        mainContent = content;
        if (mainContent) {
            mainContent->style.setWidthPercent(100.0f);
            mainContent->style.setHeightPercent(100.0f);
            addChild(mainContent);
        }
    }

    void addOverlay(FlexNode::Ptr overlay) {
        addChild(overlay);
    }

    void removeOverlay(FlexNode::Ptr overlay) {
        removeChild(overlay);
    }

    // Capture all clicks that don't hit an overlay to close them?
    bool onMouseDown(float x, float y) override {
        bool handled = FlexNode::onMouseDown(x, y);
        // If not handled by any child (overlay), we might want to close overlays
        if (!handled) {
            // Close logic could go here
        }
        return handled;
    }

private:
    FlexNode::Ptr mainContent;
};

} // namespace MochiUI
