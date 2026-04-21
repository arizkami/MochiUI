#pragma once
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <algorithm>
#include <include/core/SkRect.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkColor.h>

enum class FlexDirection { Row, Column };
enum class SizingMode { 
    Fixed,  // Use explicit width/height
    Flex,   // Fill remaining space
    Hug     // Shrink to fit children
};

struct LayoutStyle {
    float width = 0;
    float height = 0;
    SizingMode widthMode = SizingMode::Flex;
    SizingMode heightMode = SizingMode::Flex;
    
    float flex = 0;
    float padding = 0;
    float margin = 0;
    SkColor backgroundColor = SK_ColorTRANSPARENT;
    float borderRadius = 0;
    
    FlexDirection flexDirection = FlexDirection::Column;
    float gap = 0;
};

struct Size {
    float width;
    float height;
};

class FlexNode {
public:
    using Ptr = std::shared_ptr<FlexNode>;

    LayoutStyle style;
    SkRect frame = SkRect::MakeEmpty();
    std::vector<Ptr> children;
    
    // Interaction state
    bool isHovered = false;
    bool isPressed = false;
    std::function<void()> onClick;

    void addChild(Ptr child) {
        children.push_back(child);
    }

    static Ptr Create() { return std::make_shared<FlexNode>(); }
    
    static Ptr Row() {
        auto node = Create();
        node->style.flexDirection = FlexDirection::Row;
        return node;
    }

    static Ptr Column() {
        auto node = Create();
        node->style.flexDirection = FlexDirection::Column;
        return node;
    }

    bool hitTest(float x, float y) {
        return frame.contains(x, y);
    }

    // Pass 1: Measure desired size
    virtual Size measure(Size available) {
        float contentW = 0;
        float contentH = 0;

        for (auto& child : children) {
            Size childSize = child->measure(available);
            if (style.flexDirection == FlexDirection::Row) {
                contentW += childSize.width + (contentW > 0 ? style.gap : 0);
                contentH = std::max(contentH, childSize.height);
            } else {
                contentH += childSize.height + (contentH > 0 ? style.gap : 0);
                contentW = std::max(contentW, childSize.width);
            }
        }

        contentW += 2 * style.padding;
        contentH += 2 * style.padding;

        float finalW = (style.widthMode == SizingMode::Fixed) ? style.width : contentW;
        float finalH = (style.heightMode == SizingMode::Fixed) ? style.height : contentH;

        return { finalW, finalH };
    }

    // Pass 2: Final Layout
    void calculateLayout(SkRect availableSpace) {
        float padding = style.padding;
        float gap = style.gap;

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

        if (children.empty()) return;

        float totalFixed = 0;
        float totalFlex = 0;
        for (auto& child : children) {
            if (child->style.flex > 0 && ((style.flexDirection == FlexDirection::Row && child->style.widthMode == SizingMode::Flex) || 
                                          (style.flexDirection == FlexDirection::Column && child->style.heightMode == SizingMode::Flex))) {
                totalFlex += child->style.flex;
            } else {
                Size childSize = child->measure({myW - 2 * padding, myH - 2 * padding});
                totalFixed += (style.flexDirection == FlexDirection::Row) ? childSize.width : childSize.height;
            }
        }

        float totalGaps = gap * (children.size() - 1);
        float remainingSpace = (style.flexDirection == FlexDirection::Row ? myW : myH) - 2 * padding - totalFixed - totalGaps;
        if (remainingSpace < 0) remainingSpace = 0;

        float currentX = frame.left() + padding;
        float currentY = frame.top() + padding;

        for (auto& child : children) {
            Size measured = child->measure({myW - 2 * padding, myH - 2 * padding});
            float childW = measured.width;
            float childH = measured.height;

            if (style.flexDirection == FlexDirection::Row) {
                if (child->style.flex > 0 && child->style.widthMode == SizingMode::Flex) {
                    childW = (child->style.flex / totalFlex) * remainingSpace;
                }
                if (child->style.heightMode == SizingMode::Flex) childH = myH - 2 * padding;
            } else {
                if (child->style.flex > 0 && child->style.heightMode == SizingMode::Flex) {
                    childH = (child->style.flex / totalFlex) * remainingSpace;
                }
                if (child->style.widthMode == SizingMode::Flex) childW = myW - 2 * padding;
            }

            child->calculateLayout(SkRect::MakeXYWH(currentX, currentY, childW, childH));
            if (style.flexDirection == FlexDirection::Row) currentX += childW + gap;
            else currentY += childH + gap;
        }
    }

    virtual void draw(SkCanvas* canvas) {
        if (SkColorGetA(style.backgroundColor) > 0) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(style.backgroundColor);
            if (style.borderRadius > 0) canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, paint);
            else canvas->drawRect(frame, paint);
        }
        for (auto& child : children) child->draw(canvas);
    }

    // Event handling
    virtual bool onMouseMove(float x, float y) {
        bool handled = false;
        bool currentlyInside = hitTest(x, y);
        
        if (isHovered != currentlyInside) {
            isHovered = currentlyInside;
            handled = true;
        }

        for (auto& child : children) {
            if (child->onMouseMove(x, y)) handled = true;
        }
        return handled;
    }

    virtual bool onMouseDown(float x, float y) {
        bool handled = false;
        if (hitTest(x, y)) {
            isPressed = true;
            if (onClick) onClick();
            handled = true;
        }

        for (auto& child : children) {
            if (child->onMouseDown(x, y)) handled = true;
        }
        return handled;
    }

    virtual void onMouseUp(float x, float y) {
        isPressed = false;
        for (auto& child : children) child->onMouseUp(x, y);
    }
    
    virtual bool onMouseWheel(float x, float y, float delta) {
        for (auto& child : children) {
            if (child->onMouseWheel(x, y, delta)) return true;
        }
        return false;
    }
};
