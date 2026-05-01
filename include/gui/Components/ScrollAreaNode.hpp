#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>

namespace AureliaUI {

class ScrollAreaNode : public FlexNode {
public:
    ScrollAreaNode();

    // Vertical (default): content is full width, height grows; thumb on the right.
    // Horizontal: content is full height, width grows from children; thumb along the bottom.
    void setHorizontal(bool h);

    bool isHorizontal() const { return horizontal; }

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
    bool horizontal = false;
    bool isDraggingScrollbar = false;
    float lastMouseX = 0;
    float lastMouseY = 0;

    SkRect getScrollbarRect() const;
    void drawScrollbars(SkCanvas* canvas);
};

} // namespace AureliaUI
