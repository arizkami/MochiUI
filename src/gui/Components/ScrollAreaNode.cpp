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

void ScrollAreaNode::calculateLayout(SkRect availableSpace) {
    // Standard FlexNode layout for self
    float myW = availableSpace.width();
    float myH = availableSpace.height();

    if (style.widthMode == SizingMode::Fixed) myW = style.width;
    else if (style.widthMode == SizingMode::Hug) {
        myW = measure({availableSpace.width(), availableSpace.height()}).width;
    }

    if (style.heightMode == SizingMode::Fixed) myH = style.height;
    else if (style.heightMode == SizingMode::Hug) {
        myH = measure({availableSpace.width(), availableSpace.height()}).height;
    }

    frame = SkRect::MakeXYWH(availableSpace.left() + style.margin, 
                             availableSpace.top() + style.margin, 
                             myW, myH);

    if (content) {
        // Measure content again to get its true size
        Size contentMeasured = content->measure({ myW, 999999.0f });
        contentWidth = contentMeasured.width;
        contentHeight = contentMeasured.height;

        // Layout content at its full size, starting at (0,0) relative to viewport
        // The draw method handles the translation
        content->calculateLayout(SkRect::MakeXYWH(frame.left(), frame.top(), 
                                                 contentWidth, contentHeight));
    }

    updateScrollBounds();
}

void ScrollAreaNode::draw(SkCanvas* canvas) {
    if (!content) return;
    
    // Save canvas state
    canvas->save();
    
    // Clip to viewport (inner frame, accounting for padding if any)
    canvas->clipRect(frame);
    
    // Translate canvas for scrolling
    // We translate relative to the frame origin
    canvas->translate(-scrollX, -scrollY);
    
    // Draw content
    content->draw(canvas);
    
    // Restore canvas
    canvas->restore();
    
    // Derive scrollbar colors from TextPrimary to ensure visibility in any theme
    SkColor trackCol = SkColorSetA(Theme::TextPrimary, 30);
    SkColor thumbCol = SkColorSetA(Theme::TextPrimary, 80);
    SkColor thumbHoverCol = SkColorSetA(Theme::TextPrimary, 140);

    // Use isHovered to control opacity if not animated
    float alpha = isHovered ? 1.0f : 0.0f;
    if (isDraggingVertical || isDraggingHorizontal) alpha = 1.0f;
    
    if (alpha < 0.01f) return;

    auto applyAlpha = [](SkColor c, float a) {
        return SkColorSetA(c, (U8CPU)(SkColorGetA(c) * a));
    };

    // Draw vertical scrollbar
    if (showVerticalScrollbar && maxScrollY > 0) {
        SkPaint trackPaint;
        trackPaint.setAntiAlias(true);
        trackPaint.setColor(applyAlpha(trackCol, alpha));
        
        SkRect trackRect = getVerticalScrollbarRect();
        canvas->drawRoundRect(trackRect, scrollbarWidth / 2, scrollbarWidth / 2, trackPaint);
        
        // Draw thumb
        SkPaint thumbPaint;
        thumbPaint.setAntiAlias(true);
        SkColor finalThumbCol = (isHoveringVertical || isDraggingVertical) ? 
                               thumbHoverCol : thumbCol;
        thumbPaint.setColor(applyAlpha(finalThumbCol, alpha));
        
        SkRect thumbRect = getVerticalThumbRect();
        canvas->drawRoundRect(thumbRect, scrollbarWidth / 2, scrollbarWidth / 2, thumbPaint);
    }
    
    // Draw horizontal scrollbar
    if (showHorizontalScrollbar && maxScrollX > 0) {
        SkPaint trackPaint;
        trackPaint.setAntiAlias(true);
        trackPaint.setColor(applyAlpha(trackCol, alpha));
        
        SkRect trackRect = getHorizontalScrollbarRect();
        canvas->drawRoundRect(trackRect, scrollbarWidth / 2, scrollbarWidth / 2, trackPaint);
        
        // Draw thumb
        SkPaint thumbPaint;
        thumbPaint.setAntiAlias(true);
        SkColor finalThumbCol = (isHoveringHorizontal || isDraggingHorizontal) ? 
                               thumbHoverCol : thumbCol;
        thumbPaint.setColor(applyAlpha(finalThumbCol, alpha));
        
        SkRect thumbRect = getHorizontalThumbRect();
        canvas->drawRoundRect(thumbRect, scrollbarWidth / 2, scrollbarWidth / 2, thumbPaint);
    }
}

bool ScrollAreaNode::onMouseDown(float x, float y) {
    if (!hitTest(x, y)) return false;

    // Check if clicking on vertical scrollbar
    if (showVerticalScrollbar && isPointInVerticalScrollbar(x, y)) {
        SkRect thumbRect = getVerticalThumbRect();
        if (thumbRect.contains(x, y)) {
            isDraggingVertical = true;
            dragStartY = y;
            dragStartScrollY = scrollY;
            return true;
        } else {
            // Clicked track, jump scroll
            SkRect trackRect = getVerticalScrollbarRect();
            float clickRatio = std::clamp((y - trackRect.top()) / trackRect.height(), 0.0f, 1.0f);
            scrollTo(scrollX, clickRatio * maxScrollY);
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
        } else {
            // Clicked track, jump scroll
            SkRect trackRect = getHorizontalScrollbarRect();
            float clickRatio = std::clamp((x - trackRect.left()) / trackRect.width(), 0.0f, 1.0f);
            scrollTo(clickRatio * maxScrollX, scrollY);
            isDraggingHorizontal = true;
            dragStartX = x;
            dragStartScrollX = scrollX;
            return true;
        }
    }
    
    // Pass to content with adjusted coordinates
    if (content) {
        // Adjust mouse coordinates for scroll offset
        float adjustedX = x + scrollX;
        float adjustedY = y + scrollY;
        return content->onMouseDown(adjustedX, adjustedY);
    }
    
    return false;
}

bool ScrollAreaNode::onMouseMove(float x, float y) {
    bool handled = false;
    
    bool currentlyInside = hitTest(x, y);
    if (isHovered != currentlyInside) {
        isHovered = currentlyInside;
        if (isHovered) onMouseEnter();
        else onMouseLeave();
        handled = true;
    }

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
        if (content->onMouseMove(adjustedX, adjustedY)) handled = true;
    }
    
    return handled || isHoveringVertical || isHoveringHorizontal;
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

void ScrollAreaNode::onMouseEnter() {
    scrollbarOpacity = 1.0f;
}

void ScrollAreaNode::onMouseLeave() {
    scrollbarOpacity = 0.0f;
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
