#include <gui/Components/ScrollAreaNode.hpp>
#include <algorithm>

namespace AureliaUI {

ScrollAreaNode::ScrollAreaNode() {
    style.setFlex(1.0f);
    style.setWidthFull();
    style.overflowHidden = true;
}

void ScrollAreaNode::setHorizontal(bool h) {
    if (horizontal == h) {
        return;
    }
    horizontal = h;
    scrollX = 0;
    scrollY = 0;
    if (content) {
        if (horizontal) {
            content->style.setWidthAuto();
            content->style.setHeightFull();
        } else {
            content->style.setWidthFull();
            content->style.setHeightAuto();
        }
        markDirty();
    }
}

void ScrollAreaNode::setContent(FlexNode::Ptr node) {
    if (content) {
        removeChild(content);
    }
    content = node;
    if (content) {
        content->style.setFlexShrink(0.0f);
        if (horizontal) {
            content->style.setWidthAuto();
            content->style.setHeightFull();
        } else {
            content->style.setHeightAuto();
            content->style.setWidthFull();
        }
        addChild(content);
    }
}

Size ScrollAreaNode::measure(Size available) {
    float w = IsUndefined(available.width) ? 100.0f : std::max(0.0f, available.width);
    float h = IsUndefined(available.height) ? 100.0f : std::max(0.0f, available.height);
    return { w, h };
}

void ScrollAreaNode::draw(SkCanvas* canvas) {
    drawSelf(canvas);

    if (!content) {
        return;
    }

    content->syncSubtreeStyles();

    if (horizontal) {
        YGNodeCalculateLayout(content->getYGNode(), YGUndefined, frame.height(), YGDirectionLTR);

        const float contentW = content->frame.width();
        const float viewW = frame.width();
        if (contentW <= viewW) {
            scrollX = 0;
        } else {
            scrollX = std::clamp(scrollX, 0.0f, contentW - viewW);
        }

        canvas->save();
        canvas->clipRect(frame);

        content->applyYogaLayout(frame.left() - scrollX, frame.top());
        content->draw(canvas);

        canvas->restore();
    } else {
        YGNodeCalculateLayout(content->getYGNode(), frame.width(), YGUndefined, YGDirectionLTR);

        const float contentH = content->frame.height();
        const float viewH = frame.height();
        if (contentH <= viewH) {
            scrollY = 0;
        } else {
            scrollY = std::clamp(scrollY, 0.0f, contentH - viewH);
        }

        canvas->save();
        canvas->clipRect(frame);

        content->applyYogaLayout(frame.left() - scrollX, frame.top() - scrollY);
        content->draw(canvas);

        canvas->restore();
    }

    drawScrollbars(canvas);
}

SkRect ScrollAreaNode::getScrollbarRect() const {
    if (!content) {
        return SkRect::MakeEmpty();
    }

    if (horizontal) {
        const float contentW = content->frame.width();
        const float viewW = frame.width();
        if (contentW <= viewW) {
            return SkRect::MakeEmpty();
        }

        float scrollbarW = std::max(20.0f, (viewW / contentW) * viewW);
        scrollbarW = std::min(scrollbarW, viewW);
        const float track = viewW - scrollbarW;
        const float thumbX = frame.left() + (scrollX / (contentW - viewW)) * track;
        const float thumbXClamped = std::clamp(thumbX, frame.left(), frame.right() - scrollbarW);

        return SkRect::MakeXYWH(thumbXClamped, frame.bottom() - 8, scrollbarW, 6);
    }

    const float contentH = content->frame.height();
    const float viewH = frame.height();

    if (contentH <= viewH) {
        return SkRect::MakeEmpty();
    }

    float scrollbarH = std::max(20.0f, (viewH / contentH) * viewH);
    scrollbarH = std::min(scrollbarH, viewH);
    float scrollbarY = frame.top() + (scrollY / (contentH - viewH)) * (viewH - scrollbarH);
    const float maxY = frame.bottom() - scrollbarH;
    scrollbarY = std::clamp(scrollbarY, frame.top(), maxY);

    return SkRect::MakeXYWH(frame.right() - 8, scrollbarY, 6, scrollbarH);
}

void ScrollAreaNode::drawScrollbars(SkCanvas* canvas) {
    SkRect rect = getScrollbarRect();
    if (rect.isEmpty()) {
        return;
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(isDraggingScrollbar ? SkColorSetA(Theme::TextSecondary, 180) : SkColorSetA(Theme::TextSecondary, 100));

    canvas->drawRoundRect(rect, 3, 3, paint);
}

bool ScrollAreaNode::onMouseWheel(float x, float y, float delta) {
    if (!content || !hitTest(x, y)) {
        return false;
    }

    if (horizontal) {
        const float contentW = content->frame.width();
        const float viewW = frame.width();
        if (contentW > viewW) {
            scrollX -= delta * 60.0f;
            scrollX = std::clamp(scrollX, 0.0f, contentW - viewW);
            return true;
        }
        return false;
    }

    const float contentH = content->frame.height();
    const float viewH = frame.height();

    if (contentH > viewH) {
        scrollY -= delta * 60.0f;
        scrollY = std::clamp(scrollY, 0.0f, contentH - viewH);
        return true;
    }
    return false;
}

bool ScrollAreaNode::onMouseDown(float x, float y) {
    SkRect sbRect = getScrollbarRect();
    if (sbRect.contains(x, y)) {
        isDraggingScrollbar = true;
        lastMouseX = x;
        lastMouseY = y;
        return true;
    }

    if (hitTest(x, y)) {
        for (auto& child : children) {
            if (child->onMouseDown(x, y)) {
                return true;
            }
        }
    }
    return false;
}

bool ScrollAreaNode::onMouseMove(float x, float y) {
    if (isDraggingScrollbar && content) {
        if (horizontal) {
            const float contentW = content->frame.width();
            const float viewW = frame.width();
            const float scrollbarW = std::max(20.0f, (viewW / contentW) * viewW);
            const float safeBarW = std::min(scrollbarW, viewW);
            const float track = viewW - safeBarW;
            if (track > 1.0f && contentW > viewW) {
                const float dx = x - lastMouseX;
                const float scrollDelta = dx * (contentW - viewW) / track;
                scrollX = std::clamp(scrollX + scrollDelta, 0.0f, contentW - viewW);
            }
            lastMouseX = x;
        } else {
            const float contentH = content->frame.height();
            const float viewH = frame.height();
            const float scrollbarH = std::max(20.0f, (viewH / contentH) * viewH);

            const float dy = y - lastMouseY;
            const float scrollDelta = dy * (contentH - viewH) / (viewH - scrollbarH);

            scrollY = std::clamp(scrollY + scrollDelta, 0.0f, contentH - viewH);
            lastMouseY = y;
        }
        return true;
    }

    return FlexNode::onMouseMove(x, y);
}

void ScrollAreaNode::onMouseUp(float x, float y) {
    isDraggingScrollbar = false;
    FlexNode::onMouseUp(x, y);
}

} // namespace AureliaUI
