#include <include/gui/Components/Popover.hpp>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkBlurTypes.h>

namespace MochiUI {

Popover::Popover(FlexNode::Ptr content, SkRect anchorRect) : content(content), anchor(anchorRect) {
    style.setPositionType(YGPositionTypeAbsolute);
    style.setWidthAuto();
    style.setHeightAuto();
    
    // Position below the anchor in window coordinates
    style.setPosition(YGEdgeLeft, anchor.left());
    style.setPosition(YGEdgeTop, anchor.bottom() + 4.0f);
    
    // Background and shadow (via border/bg)
    style.backgroundColor = Theme::Card;
    style.borderRadius = 8.0f;
    style.setPadding(4.0f);
    
    addChild(content);
}

void Popover::calculateLayout(SkRect availableSpace) {
    // Popovers are positioned in absolute window coordinates, 
    // so we reset the parent-provided offset to 0,0.
    syncSubtreeStyles();
    YGNodeCalculateLayout(getYGNode(), YGUndefined, YGUndefined, YGDirectionLTR);
    
    // We use the absolute positions set in style.setPosition directly
    float left = YGNodeLayoutGetLeft(getYGNode());
    float top = YGNodeLayoutGetTop(getYGNode());
    float width = YGNodeLayoutGetWidth(getYGNode());
    float height = YGNodeLayoutGetHeight(getYGNode());
    frame = SkRect::MakeXYWH(left, top, width, height);
    
    for (auto& child : children) {
        child->applyYogaLayout(left, top);
    }
    clearLayoutDirtyRecursive();
}

void Popover::draw(SkCanvas* canvas) {
    // Draw a shadow/dim background for the whole screen?
    // No, just the popover for now.
    
    // Border/Shadow effect
    SkPaint shadowPaint;
    shadowPaint.setColor(Theme::Shadow);
    shadowPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 4.0f));
    canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, shadowPaint);

    drawSelf(canvas);
    
    // Draw border
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

} // namespace MochiUI
