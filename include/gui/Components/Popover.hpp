#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <functional>

namespace AureliaUI {

class Popover : public FlexNode {
public:
    // anchorRect: the widget the popover attaches to, in window coordinates.
    // screenBounds: optional window rect used for screen-edge detection.
    //   Pass an empty rect (default) to disable screen-edge clamping.
    Popover(FlexNode::Ptr content, SkRect anchorRect,
            SkRect screenBounds = SkRect::MakeEmpty());

    void calculateLayout(SkRect availableSpace) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;

    std::function<void()> onClose;

    // Update after construction if window was resized.
    // Provide window dimensions so the popover can flip/clamp to stay on screen.
    void setScreenBounds(SkRect bounds) { screenBounds = bounds; }

private:
    FlexNode::Ptr content;
    SkRect anchor;
    SkRect screenBounds;

    void clampToScreen();
};

} // namespace AureliaUI
