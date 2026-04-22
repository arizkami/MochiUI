#include <include/gui/Components/ScrollAreaNode.hpp>
#include <algorithm>
#include <windows.h>

namespace MochiUI {

void ScrollAreaNode::setContent(FlexNode::Ptr node) {
    content = node;
    if (content) {
        children.clear();
        children.push_back(content);
    }
}

void ScrollAreaNode::scrollTo(float x, float y, bool smooth) {
    targetScrollX = std::clamp(x, 0.0f, maxScrollX);
    targetScrollY = std::clamp(y, 0.0f, maxScrollY);
    if (!smooth) {
        scrollX = targetScrollX;
        scrollY = targetScrollY;
    }
}

void ScrollAreaNode::scrollBy(float dx, float dy, bool smooth) {
    scrollTo(targetScrollX + dx, targetScrollY + dy, smooth);
}

Size ScrollAreaNode::measure(Size available) {
    if (!content) return { 0, 0 };
    
    // Measure content with unlimited space in the scrolling directions
    float measureW = showHorizontalScrollbar ? 999999.0f : available.width;
    float measureH = showVerticalScrollbar ? 999999.0f : available.height;
    
    Size contentSize = content->measure({ measureW, measureH });
    contentWidth = contentSize.width;
    contentHeight = contentSize.height;
    
    float w = available.width;
    float h = available.height;
    
    if (style.widthMode == SizingMode::Fixed) w = style.width;
    else if (style.widthMode == SizingMode::Hug) w = contentWidth;

    if (style.heightMode == SizingMode::Fixed) h = style.height;
    else if (style.heightMode == SizingMode::Hug) h = contentHeight;
    
    return { w, h };
}

void ScrollAreaNode::calculateLayout(SkRect availableSpace) {
    // Standard FlexNode layout for self
    float myW = availableSpace.width();
    float myH = availableSpace.height();

    if (style.widthMode == SizingMode::Fixed) myW = style.width;
    else if (style.widthMode == SizingMode::Hug) {
        Size measured = measure({availableSpace.width(), availableSpace.height()});
        myW = measured.width;
    }

    if (style.heightMode == SizingMode::Fixed) myH = style.height;
    else if (style.heightMode == SizingMode::Hug) {
        Size measured = measure({availableSpace.width(), availableSpace.height()});
        myH = measured.height;
    }

    frame = SkRect::MakeXYWH(availableSpace.left() + style.margin, 
                             availableSpace.top() + style.margin, 
                             myW, myH);

    if (content) {
        // Layout content at its measured size
        content->calculateLayout(SkRect::MakeXYWH(frame.left(), frame.top(), 
                                                 contentWidth, contentHeight));
    }

    updateScrollBounds();
}

void ScrollAreaNode::draw(SkCanvas* canvas) {
    if (!content) return;

    // Smooth scroll interpolation
    const float lerpFactor = 0.15f;
    if (std::abs(targetScrollX - scrollX) > 0.01f) {
        scrollX += (targetScrollX - scrollX) * lerpFactor;
    } else {
        scrollX = targetScrollX;
    }

    if (std::abs(targetScrollY - scrollY) > 0.01f) {
        scrollY += (targetScrollY - scrollY) * lerpFactor;
    } else {
        scrollY = targetScrollY;
    }
    
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
    
    // Fade logic
    if (isHovered || isDraggingVertical || isDraggingHorizontal) {
        scrollbarOpacity = std::min(1.0f, scrollbarOpacity + 0.1f);
    } else {
        scrollbarOpacity = std::max(0.0f, scrollbarOpacity - 0.05f);
    }

    if (scrollbarOpacity <= 0.0f) return;

    auto applyAlpha = [this](SkColor c) {
        return SkColorSetA(c, (U8CPU)(SkColorGetA(c) * scrollbarOpacity));
    };

    SkColor thumbCol = SkColorSetA(Theme::TextPrimary, 60);
    SkColor thumbHoverCol = SkColorSetA(Theme::TextPrimary, 100);

    // Draw vertical scrollbar
    if (showVerticalScrollbar && maxScrollY > 0) {
        SkPaint thumbPaint;
        thumbPaint.setAntiAlias(true);
        SkColor finalThumbCol = (isHoveringVertical || isDraggingVertical) ? 
                               thumbHoverCol : thumbCol;
        thumbPaint.setColor(applyAlpha(finalThumbCol));
        
        SkRect thumbRect = getVerticalThumbRect();
        float radius = thumbRect.width() / 2.0f;
        canvas->drawRoundRect(thumbRect, radius, radius, thumbPaint);
    }
    
    // Draw horizontal scrollbar
    if (showHorizontalScrollbar && maxScrollX > 0) {
        SkPaint thumbPaint;
        thumbPaint.setAntiAlias(true);
        SkColor finalThumbCol = (isHoveringHorizontal || isDraggingHorizontal) ? 
                               thumbHoverCol : thumbCol;
        thumbPaint.setColor(applyAlpha(finalThumbCol));
        
        SkRect thumbRect = getHorizontalThumbRect();
        float radius = thumbRect.height() / 2.0f;
        canvas->drawRoundRect(thumbRect, radius, radius, thumbPaint);
    }
}

bool ScrollAreaNode::needsRedraw() {
    bool animating = std::abs(targetScrollX - scrollX) > 0.01f || 
                     std::abs(targetScrollY - scrollY) > 0.01f;

    if (isHovered || isDraggingVertical || isDraggingHorizontal) {
        if (scrollbarOpacity < 1.0f) animating = true;
    } else {
        if (scrollbarOpacity > 0.0f) animating = true;
    }

    if (animating) return true;
    return FlexNode::needsRedraw();
}

bool ScrollAreaNode::onMouseDown(float x, float y) {
    if (!hitTest(x, y)) return false;

    if (showVerticalScrollbar && isPointInVerticalScrollbar(x, y)) {
        SkRect thumbRect = getVerticalThumbRect();
        if (thumbRect.contains(x, y)) {
            isDraggingVertical = true;
            dragStartY = y;
            dragStartScrollY = scrollY;
            targetScrollY = scrollY;
            return true;
        } else {
            SkRect trackRect = getVerticalScrollbarRect();
            float clickRatio = std::clamp((y - trackRect.top()) / trackRect.height(), 0.0f, 1.0f);
            scrollTo(targetScrollX, clickRatio * maxScrollY, true);
            return true;
        }
    }
    
    if (showHorizontalScrollbar && isPointInHorizontalScrollbar(x, y)) {
        SkRect thumbRect = getHorizontalThumbRect();
        if (thumbRect.contains(x, y)) {
            isDraggingHorizontal = true;
            dragStartX = x;
            dragStartScrollX = scrollX;
            targetScrollX = scrollX;
            return true;
        } else {
            SkRect trackRect = getHorizontalScrollbarRect();
            float clickRatio = std::clamp((x - trackRect.left()) / trackRect.width(), 0.0f, 1.0f);
            scrollTo(clickRatio * maxScrollX, targetScrollY, true);
            return true;
        }
    }
    
    if (content) {
        float adjustedX = x + scrollX;
        float adjustedY = y + scrollY;
        return content->onMouseDown(adjustedX, adjustedY);
    }
    
    return false;
}

bool ScrollAreaNode::onMouseMove(float x, float y) {
    bool handled = FlexNode::onMouseMove(x, y);

    bool wasHoveringVertical = isHoveringVertical;
    bool wasHoveringHorizontal = isHoveringHorizontal;
    isHoveringVertical = isPointInVerticalScrollbar(x, y);
    isHoveringHorizontal = isPointInHorizontalScrollbar(x, y);
    
    if (wasHoveringVertical != isHoveringVertical || wasHoveringHorizontal != isHoveringHorizontal) {
        handled = true;
    }

    if (isDraggingVertical) {
        float delta = y - dragStartY;
        SkRect trackRect = getVerticalScrollbarRect();
        float trackHeight = trackRect.height();
        float thumbHeight = getVerticalThumbRect().height();
        float scrollableTrack = trackHeight - thumbHeight;
        
        if (scrollableTrack > 0) {
            float scrollDelta = (delta / scrollableTrack) * maxScrollY;
            scrollTo(targetScrollX, dragStartScrollY + scrollDelta, false);
        }
        return true;
    }
    
    if (isDraggingHorizontal) {
        float delta = x - dragStartX;
        SkRect trackRect = getHorizontalScrollbarRect();
        float trackWidth = trackRect.width();
        float thumbWidth = getHorizontalThumbRect().width();
        float scrollableTrack = trackWidth - thumbWidth;
        
        if (scrollableTrack > 0) {
            float scrollDelta = (delta / scrollableTrack) * maxScrollX;
            scrollTo(dragStartScrollX + scrollDelta, targetScrollY, false);
        }
        return true;
    }
    
    if (content) {
        float adjustedX = x + scrollX;
        float adjustedY = y + scrollY;
        if (content->onMouseMove(adjustedX, adjustedY)) handled = true;
    }
    
    return handled;
}

void ScrollAreaNode::onMouseUp(float x, float y) {
    isDraggingVertical = false;
    isDraggingHorizontal = false;
    
    if (content) {
        float adjustedX = x + scrollX;
        float adjustedY = y + scrollY;
        content->onMouseUp(adjustedX, adjustedY);
    }
}

void ScrollAreaNode::onMouseEnter() {
}

void ScrollAreaNode::onMouseLeave() {
}

bool ScrollAreaNode::onMouseWheel(float x, float y, float delta) {
    if (hitTest(x, y)) {
        bool shiftPressed = GetKeyState(VK_SHIFT) & 0x8000;
        float scrollAmount = delta * 100.0f;
        
        if (shiftPressed || !showVerticalScrollbar) {
            scrollBy(-scrollAmount, 0);
        } else {
            scrollBy(0, -scrollAmount);
        }
        return true;
    }
    return false;
}

void ScrollAreaNode::updateScrollBounds() {
    maxScrollX = std::max(0.0f, contentWidth - frame.width());
    maxScrollY = std::max(0.0f, contentHeight - frame.height());
    
    scrollX = std::clamp(scrollX, 0.0f, maxScrollX);
    scrollY = std::clamp(scrollY, 0.0f, maxScrollY);
    targetScrollX = std::clamp(targetScrollX, 0.0f, maxScrollX);
    targetScrollY = std::clamp(targetScrollY, 0.0f, maxScrollY);
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
