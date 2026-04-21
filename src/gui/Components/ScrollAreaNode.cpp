#include <include/gui/Components/ScrollAreaNode.hpp>
#include <algorithm>

namespace MochiUI {

void ScrollAreaNode::setContent(FlexNode::Ptr node) {
    content = node;
    if (content) {
        children.clear();
        children.push_back(content);
    }
}

void ScrollAreaNode::scrollTo(float x, float y) {
    scrollX = std::clamp(x, 0.0f, maxScrollX);
    scrollY = std::clamp(y, 0.0f, maxScrollY);
}

void ScrollAreaNode::scrollBy(float dx, float dy) {
    scrollTo(scrollX + dx, scrollY + dy);
}

Size ScrollAreaNode::measure(Size available) {
    if (!content) return { 0, 0 };
    
    // Measure content with unlimited space
    Size contentSize = content->measure({ 999999.0f, 999999.0f });
    contentWidth = contentSize.width;
    contentHeight = contentSize.height;
    
    // Return the available size (we'll scroll the content)
    float w = available.width;
    float h = available.height;
    
    if (style.widthMode == SizingMode::Fixed) w = style.width;
    if (style.heightMode == SizingMode::Fixed) h = style.height;
    
    return { w, h };
}

void ScrollAreaNode::layout(SkRect rect) {
    frame = rect;
    
    if (!content) return;
    
    // Update scroll bounds based on frame size
    updateScrollBounds();
}

void ScrollAreaNode::draw(SkCanvas* canvas) {
    if (!content) return;
    
    // Save canvas state
    canvas->save();
    
    // Clip to viewport
    canvas->clipRect(frame);
    
    // Translate canvas for scrolling
    canvas->translate(-scrollX, -scrollY);
    
    // Draw content
    content->draw(canvas);
    
    // Restore canvas
    canvas->restore();
    
    // Draw vertical scrollbar
    if (showVerticalScrollbar && maxScrollY > 0) {
        SkPaint trackPaint;
        trackPaint.setAntiAlias(true);
        trackPaint.setColor(scrollbarTrackColor);
        
        SkRect trackRect = getVerticalScrollbarRect();
        SkRRect trackRRect = SkRRect::MakeRectXY(trackRect, scrollbarWidth / 2, scrollbarWidth / 2);
        canvas->drawRRect(trackRRect, trackPaint);
        
        // Draw thumb
        SkPaint thumbPaint;
        thumbPaint.setAntiAlias(true);
        thumbPaint.setColor(isHoveringVertical || isDraggingVertical ? 
                           scrollbarThumbHoverColor : scrollbarThumbColor);
        
        SkRect thumbRect = getVerticalThumbRect();
        SkRRect thumbRRect = SkRRect::MakeRectXY(thumbRect, scrollbarWidth / 2, scrollbarWidth / 2);
        canvas->drawRRect(thumbRRect, thumbPaint);
    }
    
    // Draw horizontal scrollbar
    if (showHorizontalScrollbar && maxScrollX > 0) {
        SkPaint trackPaint;
        trackPaint.setAntiAlias(true);
        trackPaint.setColor(scrollbarTrackColor);
        
        SkRect trackRect = getHorizontalScrollbarRect();
        SkRRect trackRRect = SkRRect::MakeRectXY(trackRect, scrollbarWidth / 2, scrollbarWidth / 2);
        canvas->drawRRect(trackRRect, trackPaint);
        
        // Draw thumb
        SkPaint thumbPaint;
        thumbPaint.setAntiAlias(true);
        thumbPaint.setColor(isHoveringHorizontal || isDraggingHorizontal ? 
                           scrollbarThumbHoverColor : scrollbarThumbColor);
        
        SkRect thumbRect = getHorizontalThumbRect();
        SkRRect thumbRRect = SkRRect::MakeRectXY(thumbRect, scrollbarWidth / 2, scrollbarWidth / 2);
        canvas->drawRRect(thumbRRect, thumbPaint);
    }
}

bool ScrollAreaNode::onMouseDown(float x, float y) {
    // Check if clicking on vertical scrollbar
    if (showVerticalScrollbar && isPointInVerticalScrollbar(x, y)) {
        SkRect thumbRect = getVerticalThumbRect();
        if (thumbRect.contains(x, y)) {
            isDraggingVertical = true;
            dragStartY = y;
            dragStartScrollY = scrollY;
            return true;
        }
    }
    
    // Check if clicking on horizontal scrollbar
    if (showHorizontalScrollbar && isPointInHorizontalScrollbar(x, y)) {
        SkRect thumbRect = getHorizontalThumbRect();
        if (thumbRect.contains(x, y)) {
            isDraggingHorizontal = true;
            dragStartX = x;
            dragStartScrollX = scrollX;
            return true;
        }
    }
    
    // Pass to content with adjusted coordinates
    if (content && hitTest(x, y)) {
        // Adjust mouse coordinates for scroll offset
        float adjustedX = x + scrollX;
        float adjustedY = y + scrollY;
        return content->onMouseDown(adjustedX, adjustedY);
    }
    
    return false;
}

bool ScrollAreaNode::onMouseMove(float x, float y) {
    // Update hover state
    isHoveringVertical = isPointInVerticalScrollbar(x, y);
    isHoveringHorizontal = isPointInHorizontalScrollbar(x, y);
    
    // Handle vertical scrollbar dragging
    if (isDraggingVertical) {
        float delta = y - dragStartY;
        SkRect trackRect = getVerticalScrollbarRect();
        float trackHeight = trackRect.height();
        float thumbHeight = getVerticalThumbRect().height();
        float scrollableTrack = trackHeight - thumbHeight;
        
        if (scrollableTrack > 0) {
            float scrollDelta = (delta / scrollableTrack) * maxScrollY;
            scrollTo(scrollX, dragStartScrollY + scrollDelta);
        }
        return true;
    }
    
    // Handle horizontal scrollbar dragging
    if (isDraggingHorizontal) {
        float delta = x - dragStartX;
        SkRect trackRect = getHorizontalScrollbarRect();
        float trackWidth = trackRect.width();
        float thumbWidth = getHorizontalThumbRect().width();
        float scrollableTrack = trackWidth - thumbWidth;
        
        if (scrollableTrack > 0) {
            float scrollDelta = (delta / scrollableTrack) * maxScrollX;
            scrollTo(dragStartScrollX + scrollDelta, scrollY);
        }
        return true;
    }
    
    // Pass to content with adjusted coordinates
    if (content) {
        float adjustedX = x + scrollX;
        float adjustedY = y + scrollY;
        return content->onMouseMove(adjustedX, adjustedY);
    }
    
    return false;
}

void ScrollAreaNode::onMouseUp(float x, float y) {
    isDraggingVertical = false;
    isDraggingHorizontal = false;
    
    if (content) {
        // Adjust mouse coordinates for scroll offset
        float adjustedX = x + scrollX;
        float adjustedY = y + scrollY;
        content->onMouseUp(adjustedX, adjustedY);
    }
}

bool ScrollAreaNode::onMouseWheel(float x, float y, float delta) {
    if (hitTest(x, y)) {
        // Scroll vertically
        scrollBy(0, -delta * 20.0f);  // Adjust scroll speed
        return true;
    }
    return false;
}

void ScrollAreaNode::updateScrollBounds() {
    maxScrollX = std::max(0.0f, contentWidth - frame.width());
    maxScrollY = std::max(0.0f, contentHeight - frame.height());
    
    // Clamp current scroll position
    scrollX = std::clamp(scrollX, 0.0f, maxScrollX);
    scrollY = std::clamp(scrollY, 0.0f, maxScrollY);
}

SkRect ScrollAreaNode::getVerticalScrollbarRect() const {
    return SkRect::MakeXYWH(
        frame.right() - scrollbarWidth - scrollbarPadding,
        frame.top() + scrollbarPadding,
        scrollbarWidth,
        frame.height() - 2 * scrollbarPadding
    );
}

SkRect ScrollAreaNode::getHorizontalScrollbarRect() const {
    return SkRect::MakeXYWH(
        frame.left() + scrollbarPadding,
        frame.bottom() - scrollbarWidth - scrollbarPadding,
        frame.width() - 2 * scrollbarPadding,
        scrollbarWidth
    );
}

SkRect ScrollAreaNode::getVerticalThumbRect() const {
    SkRect trackRect = getVerticalScrollbarRect();
    float viewportHeight = frame.height();
    float thumbHeight = std::max(20.0f, (viewportHeight / contentHeight) * trackRect.height());
    float scrollRatio = maxScrollY > 0 ? (scrollY / maxScrollY) : 0.0f;
    float thumbY = trackRect.top() + scrollRatio * (trackRect.height() - thumbHeight);
    
    return SkRect::MakeXYWH(
        trackRect.left(),
        thumbY,
        scrollbarWidth,
        thumbHeight
    );
}

SkRect ScrollAreaNode::getHorizontalThumbRect() const {
    SkRect trackRect = getHorizontalScrollbarRect();
    float viewportWidth = frame.width();
    float thumbWidth = std::max(20.0f, (viewportWidth / contentWidth) * trackRect.width());
    float scrollRatio = maxScrollX > 0 ? (scrollX / maxScrollX) : 0.0f;
    float thumbX = trackRect.left() + scrollRatio * (trackRect.width() - thumbWidth);
    
    return SkRect::MakeXYWH(
        thumbX,
        trackRect.top(),
        thumbWidth,
        scrollbarWidth
    );
}

bool ScrollAreaNode::isPointInVerticalScrollbar(float x, float y) const {
    if (!showVerticalScrollbar || maxScrollY <= 0) return false;
    return getVerticalScrollbarRect().contains(x, y);
}

bool ScrollAreaNode::isPointInHorizontalScrollbar(float x, float y) const {
    if (!showHorizontalScrollbar || maxScrollX <= 0) return false;
    return getHorizontalScrollbarRect().contains(x, y);
}

} // namespace MochiUI
