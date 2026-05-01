#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>

namespace AureliaUI {

class ModalNode : public FlexNode {
public:
    ModalNode(FlexNode::Ptr content) {
        style.setWidthPercent(100.0f);
        style.setHeightPercent(100.0f);
        style.setPositionType(YGPositionTypeAbsolute);
        style.backgroundColor = AUKColor::black().withAlpha(uint8_t(150)); // Dim background
        style.setAlignItems(YGAlignCenter);

        container = FlexNode::Create();
        container->style.backgroundColor = Theme::Background;
        container->style.setPadding(20);
        container->style.borderRadius = 8;
        container->style.setWidthAuto();
        container->style.setHeightAuto();
        container->style.setMinWidth(300);

        container->addChild(content);
        addChild(container);
    }

    bool onMouseDown(float x, float y) override {
        if (!container->hitTest(x, y)) {
            // Clicked outside content - could close modal
            // return true to consume and prevent clicks through to main app
        }
        return FlexNode::onMouseDown(x, y);
    }

private:
    FlexNode::Ptr container;
};

} // namespace AureliaUI
