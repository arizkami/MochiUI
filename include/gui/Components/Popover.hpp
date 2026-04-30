#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <functional>

namespace MochiUI {

class Popover : public FlexNode {
public:
    Popover(FlexNode::Ptr content, SkRect anchorRect);
    
    void calculateLayout(SkRect availableSpace) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;

    std::function<void()> onClose;

private:
    FlexNode::Ptr content;
    SkRect anchor;
};

} // namespace MochiUI
