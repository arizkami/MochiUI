#include <gui/Components/Popover.hpp>
#include <algorithm>

namespace SphereUI {

Popover::Popover(FlexNode::Ptr content, SkRect anchorRect, SkRect screenBounds)
    : content(content), anchor(anchorRect), screenBounds(screenBounds)
{
    style.setPositionType(YGPositionTypeAbsolute);
    style.setWidthAuto();
    style.setHeightAuto();

    // Default: open below the anchor
    style.setPosition(YGEdgeLeft, anchor.left());
    style.setPosition(YGEdgeTop, anchor.bottom() + 6.0f);

    style.backgroundColor = Theme::Card;
    style.borderRadius = 8.0f;
    style.setPadding(4.0f);

    addChild(content);
}

void Popover::calculateLayout(SkRect availableSpace) {
    syncSubtreeStyles();
    YGNodeCalculateLayout(getYGNode(), YGUndefined, YGUndefined, YGDirectionLTR);

    float left   = YGNodeLayoutGetLeft(getYGNode());
    float top    = YGNodeLayoutGetTop(getYGNode());
    float width  = YGNodeLayoutGetWidth(getYGNode());
    float height = YGNodeLayoutGetHeight(getYGNode());
    frame = SkRect::MakeXYWH(left, top, width, height);

    clampToScreen();

    for (auto& child : children) {
        child->applyYogaLayout(frame.left(), frame.top());
    }
    clearLayoutDirtyRecursive();
}

void Popover::clampToScreen() {
    // Only clamp when caller supplied valid screen bounds
    if (screenBounds.isEmpty()) return;

    float newLeft = frame.left();
    float newTop  = frame.top();

    // Flip above anchor if popover would go below screen
    if (frame.bottom() > screenBounds.bottom()) {
        newTop = anchor.top() - frame.height() - 6.0f;
    }
    // Shift left if right edge overflows
    if (newLeft + frame.width() > screenBounds.right()) {
        newLeft = screenBounds.right() - frame.width();
    }
    // Clamp to left / top screen edges
    newLeft = std::max(screenBounds.left(), newLeft);
    newTop  = std::max(screenBounds.top(), newTop);

    frame.offsetTo(newLeft, newTop);
}

void Popover::draw(SkCanvas* canvas) {
    // Drop shadow (offset so it appears beneath the popover)
    SkPaint shadowPaint;
    shadowPaint.setColor(Theme::Shadow);
    shadowPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 8.0f));
    SkRect shadowFrame = frame.makeOffset(0.0f, 4.0f);
    canvas->drawRoundRect(shadowFrame, style.borderRadius, style.borderRadius, shadowPaint);

    drawSelf(canvas);

    // Border
    SkPaint borderPaint;
    borderPaint.setAntiAlias(true);
    borderPaint.setStyle(SkPaint::kStroke_Style);
    borderPaint.setColor(Theme::Border);
    borderPaint.setStrokeWidth(1.0f);
    canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, borderPaint);

    drawChildren(canvas);
}

bool Popover::onMouseDown(float x, float y) {
    if (!hitTest(x, y)) {
        if (onClose) onClose();
        return false; // Let it fall through so something else can be clicked
    }
    return FlexNode::onMouseDown(x, y);
}

} // namespace SphereUI
