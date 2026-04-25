#include <include/gui/Components/ScrollAreaNode.hpp>
#include <algorithm>

namespace MochiUI {

ScrollAreaNode::ScrollAreaNode() {
    style.setFlex(1.0f);
    style.overflowHidden = true;
}

void ScrollAreaNode::setContent(FlexNode::Ptr node) {
    if (content) {
        removeChild(content);
    }
    content = node;
    if (content) {
        // Essential for scrolling: don't shrink content to fit view, 
        // and allow it to be as tall as its children need.
        content->style.setFlexShrink(0.0f);
        content->style.setHeightAuto();
        content->style.setWidthFull();
        addChild(content);
    }
}

Size ScrollAreaNode::measure(Size available) {
    return { 100, 100 };
}

void ScrollAreaNode::draw(SkCanvas* canvas) {
    drawSelf(canvas);
    
    if (!content) return;

    // Ensure layout is up to date for the scroll content
    content->syncSubtreeStyles();
    YGNodeCalculateLayout(content->getYGNode(), frame.width(), YGUndefined, YGDirectionLTR);

    canvas->save();
    canvas->clipRect(frame);
    
    // Update content layout relative to scroll
    content->applyYogaLayout(frame.left() - scrollX, frame.top() - scrollY);
    content->draw(canvas);
    
    canvas->restore();

    drawScrollbars(canvas);
}

SkRect ScrollAreaNode::getScrollbarRect() const {
    if (!content) return SkRect::MakeEmpty();

    float contentH = content->frame.height();
    float viewH = frame.height();
    
    if (contentH <= viewH) return SkRect::MakeEmpty();

    float scrollbarH = std::max(20.0f, (viewH / contentH) * viewH);
    float scrollbarY = frame.top() + (scrollY / (contentH - viewH)) * (viewH - scrollbarH);
    
    return SkRect::MakeXYWH(frame.right() - 8, scrollbarY, 6, scrollbarH);
}

void ScrollAreaNode::drawScrollbars(SkCanvas* canvas) {
    SkRect rect = getScrollbarRect();
    if (rect.isEmpty()) return;

    SkPaint paint;
    paint.setAntiAlias(true);
    // Darker if dragging
    paint.setColor(isDraggingScrollbar ? SkColorSetA(Theme::TextSecondary, 180) : SkColorSetA(Theme::TextSecondary, 100));
    
    canvas->drawRoundRect(rect, 3, 3, paint);
}

bool ScrollAreaNode::onMouseWheel(float x, float y, float delta) {
    if (!content || !hitTest(x, y)) return false;

    float contentH = content->frame.height();
    float viewH = frame.height();
    
    if (contentH > viewH) {
        scrollY -= delta * 60.0f; // Faster scroll
        scrollY = std::clamp(scrollY, 0.0f, contentH - viewH);
        return true;
    }
    return false;
}

bool ScrollAreaNode::onMouseDown(float x, float y) {
    SkRect sbRect = getScrollbarRect();
    if (sbRect.contains(x, y)) {
        isDraggingScrollbar = true;
        lastMouseY = y;
        return true;
    }
    
    // Offset coordinates for children since we draw them with an offset
    if (hitTest(x, y)) {
        for (auto& child : children) {
            if (child->onMouseDown(x, y)) return true;
        }
    }
    return false;
}

bool ScrollAreaNode::onMouseMove(float x, float y) {
    if (isDraggingScrollbar && content) {
        float contentH = content->frame.height();
        float viewH = frame.height();
        float scrollbarH = std::max(20.0f, (viewH / contentH) * viewH);
        
        float dy = y - lastMouseY;
        float scrollDelta = dy * (contentH - viewH) / (viewH - scrollbarH);
        
        scrollY = std::clamp(scrollY + scrollDelta, 0.0f, contentH - viewH);
        lastMouseY = y;
        return true;
    }
    
    return FlexNode::onMouseMove(x, y);
}

void ScrollAreaNode::onMouseUp(float x, float y) {
    isDraggingScrollbar = false;
    FlexNode::onMouseUp(x, y);
}

} // namespace MochiUI
