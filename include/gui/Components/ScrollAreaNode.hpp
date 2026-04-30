#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>

namespace MochiUI {

class ScrollAreaNode : public FlexNode {
public:
    ScrollAreaNode();
    
    void setContent(FlexNode::Ptr node);
    void draw(SkCanvas* canvas) override;
    Size measure(Size available) override;
    bool onMouseWheel(float x, float y, float delta) override;
    bool onMouseDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;
    void onMouseUp(float x, float y) override;

private:
    FlexNode::Ptr content;
    float scrollX = 0;
    float scrollY = 0;
    bool isDraggingScrollbar = false;
    float lastMouseY = 0;
    
    SkRect getScrollbarRect() const;
    void drawScrollbars(SkCanvas* canvas);
};

} // namespace MochiUI
