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
#include <External/yoga/yoga/Yoga.h>

namespace MochiUI {

enum class FlexDirection { Row, Column };
enum class SizingMode { Fixed, Flex, Hug };
enum class AlignItems { FlexStart, Center, FlexEnd, Stretch };

class FlexNode;

class LayoutStyle {
public:
    FlexNode* owner = nullptr;
    
    void setWidth(float w) { width = w; widthMode = SizingMode::Fixed; }
    void setHeight(float h) { height = h; heightMode = SizingMode::Fixed; }
    void setWidthFull() { widthMode = SizingMode::Flex; widthPercent = 100.0f; }
    void setHeightFull() { heightMode = SizingMode::Flex; heightPercent = 100.0f; }
    void setWidthPercent(float p) { widthMode = SizingMode::Flex; widthPercent = p; }
    void setHeightPercent(float p) { heightMode = SizingMode::Flex; heightPercent = p; }
    void setWidthAuto() { widthMode = SizingMode::Hug; }
    void setHeightAuto() { heightMode = SizingMode::Hug; }
    
    void setFlex(float f) { flex = f; }
    void setFlexGrow(float f) { flex = f; }
    void setFlexShrink(float f) { flexShrink = f; }
    void setFlexBasis(float f) { flexBasis = f; }

    void setPadding(float p) { padding = p; }
    void setMargin(float m) { margin = m; }
    void setGap(float g) { gap = g; }
    
    void setFlexDirection(YGFlexDirection dir) { 
        flexDirection = (dir == YGFlexDirectionRow) ? FlexDirection::Row : FlexDirection::Column;
    }
    void setAlignItems(YGAlign align) { alignItems = (AlignItems)align; }
    void setFlexWrap(YGWrap wrap) { flexWrap = wrap; }
    void setPositionType(YGPositionType type) { positionType = type; }
    void setPosition(YGEdge edge, float value) {
        if (edge == YGEdgeLeft) left = value;
        else if (edge == YGEdgeTop) top = value;
        else if (edge == YGEdgeRight) right = value;
        else if (edge == YGEdgeBottom) bottom = value;
    }

    SkColor backgroundColor = SK_ColorTRANSPARENT;
    float borderRadius = 0;
    bool overflowHidden = false;

    void syncLegacy() {
        YGNodeRef node = getYGNode();
        
        if (widthMode == SizingMode::Fixed) {
            YGNodeStyleSetWidth(node, width);
        } else if (widthMode == SizingMode::Flex) {
            YGNodeStyleSetWidthPercent(node, widthPercent);
        } else {
            YGNodeStyleSetWidthAuto(node);
        }

        if (heightMode == SizingMode::Fixed) {
            YGNodeStyleSetHeight(node, height);
        } else if (heightMode == SizingMode::Flex) {
            YGNodeStyleSetHeightPercent(node, heightPercent);
        } else {
            YGNodeStyleSetHeightAuto(node);
        }

        YGNodeStyleSetFlexGrow(node, flex);
        YGNodeStyleSetFlexShrink(node, flexShrink);
        
        if (flexBasis < 0) YGNodeStyleSetFlexBasisAuto(node);
        else YGNodeStyleSetFlexBasis(node, flexBasis);
        
        YGNodeStyleSetPadding(node, YGEdgeAll, padding);
        YGNodeStyleSetMargin(node, YGEdgeAll, margin);
        YGNodeStyleSetGap(node, YGGutterAll, gap);

        YGNodeStyleSetFlexDirection(node, flexDirection == FlexDirection::Row ? YGFlexDirectionRow : YGFlexDirectionColumn);

        YGAlign align = YGAlignStretch;
        if (alignItems == AlignItems::Center) align = YGAlignCenter;
        else if (alignItems == AlignItems::FlexEnd) align = YGAlignFlexEnd;
        else if (alignItems == AlignItems::FlexStart) align = YGAlignFlexStart;
        YGNodeStyleSetAlignItems(node, align);

        YGNodeStyleSetFlexWrap(node, flexWrap);
        YGNodeStyleSetPositionType(node, positionType);
        if (positionType == YGPositionTypeAbsolute) {
            if (left != -1e9f) YGNodeStyleSetPosition(node, YGEdgeLeft, left);
            if (top != -1e9f) YGNodeStyleSetPosition(node, YGEdgeTop, top);
            if (right != -1e9f) YGNodeStyleSetPosition(node, YGEdgeRight, right);
            if (bottom != -1e9f) YGNodeStyleSetPosition(node, YGEdgeBottom, bottom);
        }
    }

    float width = 0;
    float height = 0;
    float widthPercent = 100.0f;
    float heightPercent = 100.0f;
    float flex = 0;
    float flexShrink = 1.0f;
    float flexBasis = -1.0f; // -1 for Auto
    float padding = 0;
    float margin = 0;
    float gap = 0;
    float left = -1e9f;
    float top = -1e9f;
    float right = -1e9f;
    float bottom = -1e9f;
    YGPositionType positionType = YGPositionTypeStatic;
    YGWrap flexWrap = YGWrapNoWrap;
    SizingMode widthMode = SizingMode::Hug;
    SizingMode heightMode = SizingMode::Hug;
    FlexDirection flexDirection = FlexDirection::Column;
    AlignItems alignItems = AlignItems::Stretch;

private:
    YGNodeRef getYGNode();
};

struct Size {
    float width;
    float height;
};

class FlexNode : public std::enable_shared_from_this<FlexNode> {
public:
    using Ptr = std::shared_ptr<FlexNode>;

    FlexNode() {
        ygNode = YGNodeNew();
        YGNodeSetContext(ygNode, this);
        style.owner = this;
    }

    virtual ~FlexNode() {
        if (ygNode) {
            YGNodeFree(ygNode);
        }
    }

    YGNodeRef getYGNode() const { return ygNode; }
    LayoutStyle style;
    SkRect frame = SkRect::MakeEmpty();
    std::vector<Ptr> children;
    FlexNode* parent = nullptr;
    
    bool isHovered = false;
    bool isPressed = false;
    bool isFocused = false;
    bool enableHover = false;
    std::function<void()> onClick;

    virtual void addChild(Ptr child) {
        if (!child) return;
        if (child->parent) {
            child->parent->removeChild(child);
        }
        child->parent = this;
        children.push_back(child);
        YGNodeInsertChild(ygNode, child->getYGNode(), (uint32_t)(children.size() - 1));
    }

    virtual void removeChild(Ptr child) {
        auto it = std::find(children.begin(), children.end(), child);
        if (it != children.end()) {
            child->parent = nullptr;
            YGNodeRemoveChild(ygNode, child->getYGNode());
            children.erase(it);
        }
    }

    virtual void removeAllChildren() {
        while (!children.empty()) {
            children.back()->parent = nullptr;
            YGNodeRemoveChild(ygNode, children.back()->getYGNode());
            children.pop_back();
        }
    }

    static Ptr Create() { return std::make_shared<FlexNode>(); }
    
    static Ptr Row() {
        auto node = Create();
        node->style.flexDirection = FlexDirection::Row;
        YGNodeStyleSetFlexDirection(node->ygNode, YGFlexDirectionRow);
        return node;
    }

    static Ptr Column() {
        auto node = Create();
        node->style.flexDirection = FlexDirection::Column;
        YGNodeStyleSetFlexDirection(node->ygNode, YGFlexDirectionColumn);
        return node;
    }

    virtual bool hitTest(float x, float y) {
        return frame.contains(x, y);
    }

    static YGSize MeasureCallback(YGNodeConstRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
        FlexNode* flexNode = (FlexNode*)YGNodeGetContext(node);
        Size available = { width, height };
        Size measured = flexNode->measure(available);
        YGSize result;
        result.width = measured.width;
        result.height = measured.height;
        return result;
    }

    virtual Size measure(Size available) {
        return { 0, 0 };
    }

    virtual void calculateLayout(SkRect availableSpace) {
        syncSubtreeStyles();
        YGNodeCalculateLayout(ygNode, availableSpace.width(), availableSpace.height(), YGDirectionLTR);
        applyYogaLayout(availableSpace.left(), availableSpace.top());
    }

    void syncSubtreeStyles() {
        style.syncLegacy();
        for (auto& child : children) {
            child->syncSubtreeStyles();
        }
    }

    void applyYogaLayout(float offsetX, float offsetY) {
        float left = YGNodeLayoutGetLeft(ygNode) + offsetX;
        float top = YGNodeLayoutGetTop(getYGNode()) + offsetY;
        float width = YGNodeLayoutGetWidth(getYGNode());
        float height = YGNodeLayoutGetHeight(getYGNode());
        frame = SkRect::MakeXYWH(left, top, width, height);
        for (auto& child : children) {
            child->applyYogaLayout(left, top);
        }
    }

    virtual void draw(SkCanvas* canvas) {
        drawSelf(canvas);
        drawChildren(canvas);
    }

    virtual void drawSelf(SkCanvas* canvas) {
        if (SkColorGetA(style.backgroundColor) > 0) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(style.backgroundColor);
            if (style.borderRadius > 0) canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, paint);
            else canvas->drawRect(frame, paint);
        }

        if (enableHover && isHovered) {
            SkPaint hoverPaint;
            hoverPaint.setAntiAlias(true);
            hoverPaint.setColor(SkColorSetARGB(40, 255, 255, 255)); // Generic light overlay
            if (style.borderRadius > 0) canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, hoverPaint);
            else canvas->drawRect(frame, hoverPaint);
        }
    }

    virtual void drawChildren(SkCanvas* canvas) {
        bool needsClip = style.overflowHidden;
        if (needsClip) {
            canvas->save();
            canvas->clipRect(frame);
        }

        for (auto& child : children) child->draw(canvas);

        if (needsClip) {
            canvas->restore();
        }
    }

    virtual bool onMouseMove(float x, float y) {
        bool handled = false;
        // Check children in reverse (top to bottom)
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            if ((*it)->onMouseMove(x, y)) {
                handled = true;
                break;
            }
        }

        bool currentlyInside = hitTest(x, y);
        if (isHovered != currentlyInside) {
            isHovered = currentlyInside;
            if (isHovered) onMouseEnter();
            else onMouseLeave();
            handled = true;
        }
        return handled;
    }

    virtual void onMouseEnter() {}
    virtual void onMouseLeave() {}

    virtual bool onMouseDown(float x, float y) {
        // Check children in reverse (top to bottom)
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            if ((*it)->onMouseDown(x, y)) return true;
        }

        if (hitTest(x, y)) {
            isPressed = true;
            isFocused = true;
            if (onClick) onClick();
            return true;
        } else {
            isFocused = false;
            return false;
        }
    }

    virtual bool onRightDown(float x, float y) {
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            if ((*it)->onRightDown(x, y)) return true;
        }
        return false;
    }

    virtual void onMouseUp(float x, float y) {
        isPressed = false;
        for (auto& child : children) child->onMouseUp(x, y);
    }
    
    virtual bool onMouseWheel(float x, float y, float delta) {
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            if ((*it)->onMouseWheel(x, y, delta)) return true;
        }
        return false;
    }

    virtual bool onKeyDown(uint32_t key) {
        for (auto& child : children) if (child->onKeyDown(key)) return true;
        return false;
    }

    virtual bool onChar(uint32_t charCode) {
        for (auto& child : children) if (child->onChar(charCode)) return true;
        return false;
    }

    virtual bool needsRedraw() {
        for (auto& child : children) if (child->needsRedraw()) return true;
        return false;
    }

private:
    YGNodeRef ygNode;
};

inline YGNodeRef LayoutStyle::getYGNode() { return owner->getYGNode(); }

} // namespace MochiUI
