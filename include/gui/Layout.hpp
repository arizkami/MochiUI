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
#include <yoga/Yoga.h>
#include <core/IWindowHost.hpp>
#include <core/events/Events.hpp>

namespace MochiUI {

enum class FlexDirection { Row, Column };
enum class SizingMode { Fixed, Flex, Hug };
enum class AlignItems { FlexStart, Center, FlexEnd, Stretch };
enum class Cursor { Arrow, IBeam, Hand, SizeNS, SizeWE };

class FlexNode;

class LayoutStyle {
public:
    FlexNode* owner = nullptr;
    Cursor cursorType = Cursor::Arrow;
    
    void setWidth(float w) { width = w; widthMode = SizingMode::Fixed; }
    void setHeight(float h) { height = h; heightMode = SizingMode::Fixed; }
    void setWidthFull() { widthMode = SizingMode::Flex; widthPercent = 100.0f; }
    void setHeightFull() { heightMode = SizingMode::Flex; heightPercent = 100.0f; }
    void setWidthPercent(float p) { widthMode = SizingMode::Flex; widthPercent = p; }
    void setHeightPercent(float p) { heightMode = SizingMode::Flex; heightPercent = p; }
    void setWidthAuto() { widthMode = SizingMode::Hug; }
    void setHeightAuto() { heightMode = SizingMode::Hug; }
    
    void setMinWidth(float w) { minWidth = w; }
    void setMinHeight(float h) { minHeight = h; }
    void setMinWidthPercent(float p) { minWidth = -1e9f; minWidthPercent = p; }
    void setMinHeightPercent(float p) { minHeight = -1e9f; minHeightPercent = p; }
    
    void setFlex(float f) { flex = f; }
    void setFlexGrow(float f) { flex = f; }
    void setFlexShrink(float f) { flexShrink = f; }
    void setFlexBasis(float f) { flexBasis = f; }

    void setPadding(float p) { paddingLeft = paddingTop = paddingRight = paddingBottom = p; }
    void setPadding(float h, float v) { paddingLeft = paddingRight = h; paddingTop = paddingBottom = v; }
    void setPadding(float l, float t, float r, float b) { paddingLeft = l; paddingTop = t; paddingRight = r; paddingBottom = b; }
    
    void setMargin(float m) { marginLeft = marginTop = marginRight = marginBottom = m; }
    void setMargin(float h, float v) { marginLeft = marginRight = h; marginTop = marginBottom = v; }
    
    void setGap(float g) { gap = g; }
    
    void setFlexDirection(YGFlexDirection dir) { 
        flexDirection = (dir == YGFlexDirectionRow) ? FlexDirection::Row : FlexDirection::Column;
    }
    void setAlignItems(YGAlign align) {
        switch (align) {
            case YGAlignCenter:   alignItems = AlignItems::Center;   break;
            case YGAlignFlexEnd:  alignItems = AlignItems::FlexEnd;  break;
            case YGAlignStretch:  alignItems = AlignItems::Stretch;  break;
            default:              alignItems = AlignItems::FlexStart; break;
        }
    }
    void setJustifyContent(YGJustify justify) { justifyContent = justify; }
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
        
        YGNodeStyleSetPadding(node, YGEdgeLeft, paddingLeft != 0 ? paddingLeft : padding);
        YGNodeStyleSetPadding(node, YGEdgeTop, paddingTop != 0 ? paddingTop : padding);
        YGNodeStyleSetPadding(node, YGEdgeRight, paddingRight != 0 ? paddingRight : padding);
        YGNodeStyleSetPadding(node, YGEdgeBottom, paddingBottom != 0 ? paddingBottom : padding);
        
        YGNodeStyleSetMargin(node, YGEdgeLeft, marginLeft != 0 ? marginLeft : margin);
        YGNodeStyleSetMargin(node, YGEdgeTop, marginTop != 0 ? marginTop : margin);
        YGNodeStyleSetMargin(node, YGEdgeRight, marginRight != 0 ? marginRight : margin);
        YGNodeStyleSetMargin(node, YGEdgeBottom, marginBottom != 0 ? marginBottom : margin);

        YGNodeStyleSetGap(node, YGGutterAll, gap);

        YGNodeStyleSetFlexDirection(node, flexDirection == FlexDirection::Row ? YGFlexDirectionRow : YGFlexDirectionColumn);
        YGNodeStyleSetJustifyContent(node, justifyContent);

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

        if (minWidth != -1e9f) YGNodeStyleSetMinWidth(node, minWidth);
        else if (minWidthPercent != 0) YGNodeStyleSetMinWidthPercent(node, minWidthPercent);
        
        if (minHeight != -1e9f) YGNodeStyleSetMinHeight(node, minHeight);
        else if (minHeightPercent != 0) YGNodeStyleSetMinHeightPercent(node, minHeightPercent);
    }

    float width = 0;
    float height = 0;
    float widthPercent = 100.0f;
    float heightPercent = 100.0f;

    float minWidth = -1e9f;
    float minHeight = -1e9f;
    float minWidthPercent = 0;
    float minHeightPercent = 0;

    float flex = 0;
    float flexShrink = 1.0f;
    float flexBasis = -1.0f; // -1 for Auto
    
    float padding = 0;
    float margin = 0;
    float gap = 0;
    
    float paddingLeft = 0;
    float paddingTop = 0;
    float paddingRight = 0;
    float paddingBottom = 0;
    
    float marginLeft = 0;
    float marginTop = 0;
    float marginRight = 0;
    float marginBottom = 0;

    float left = -1e9f;
    float top = -1e9f;
    float right = -1e9f;
    float bottom = -1e9f;
    YGPositionType positionType = YGPositionTypeStatic;
    YGWrap flexWrap = YGWrapNoWrap;
    YGJustify justifyContent = YGJustifyFlexStart;
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

inline bool IsUndefined(float value) {
    return YGFloatIsUndefined(value);
}

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
    
    bool dirtyLayout = true;
    void markDirty() {
        dirtyLayout = true;
        if (parent) parent->markDirty();
        requestRedraw();
    }
    
    IWindowHost* windowHost = nullptr;
    virtual void setWindowHost(IWindowHost* host) {
        windowHost = host;
        for (auto& child : children) child->setWindowHost(host);
    }

    virtual void requestRedraw() {
        if (windowHost) windowHost->requestRedraw();
    }
    
    bool isHovered = false;
    bool isPressed = false;
    bool isFocused = false;
    bool enableHover = false;
    std::function<void()> onClick;

    float getLayoutPadding(YGEdge edge) const { return YGNodeLayoutGetPadding(ygNode, edge); }
    float getLayoutMargin(YGEdge edge) const { return YGNodeLayoutGetMargin(ygNode, edge); }

    virtual void addChild(Ptr child) {
        if (!child) return;
        if (child->parent) {
            child->parent->removeChild(child);
        }
        child->parent = this;
        child->setWindowHost(windowHost);
        children.push_back(child);
        YGNodeInsertChild(ygNode, child->getYGNode(), (uint32_t)(children.size() - 1));
        markDirty();
    }

    virtual void removeChild(Ptr child) {
        auto it = std::find(children.begin(), children.end(), child);
        if (it != children.end()) {
            child->parent = nullptr;
            YGNodeRemoveChild(ygNode, child->getYGNode());
            children.erase(it);
            markDirty();
        }
    }

    virtual void removeAllChildren() {
        if (children.empty()) return;
        while (!children.empty()) {
            children.back()->parent = nullptr;
            YGNodeRemoveChild(ygNode, children.back()->getYGNode());
            children.pop_back();
        }
        markDirty();
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

    virtual Ptr findNodeAt(float x, float y) {
        // Search children in reverse (top to bottom)
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            auto found = (*it)->findNodeAt(x, y);
            if (found) return found;
        }

        if (hitTest(x, y)) {
            try {
                return shared_from_this();
            } catch (const std::bad_weak_ptr&) {
                return nullptr;
            }
        }
        return nullptr;
    }

    static YGSize MeasureCallback(YGNodeConstRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
        FlexNode* flexNode = (FlexNode*)YGNodeGetContext(node);
        Size available = { width, height };
        
        // Convert Yoga Undefined to something easier to handle if needed
        // But passing it as is (NAN) is also okay if measure handles it.
        
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
        clearLayoutDirtyRecursive();
    }

    void clearLayoutDirtyRecursive() {
        dirtyLayout = false;
        for (auto& child : children) {
            child->clearLayoutDirtyRecursive();
        }
    }

    virtual void syncSubtreeStyles() {
        style.syncLegacy();
        for (auto& child : children) {
            child->syncSubtreeStyles();
        }
    }

    virtual void applyYogaLayout(float offsetX, float offsetY) {
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
        bool handled = false;
        // Check children in reverse (top to bottom)
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            if ((*it)->onMouseDown(x, y)) {
                handled = true;
                break;
            }
        }

        if (handled) {
            isFocused = false; // Someone else (a child) took focus
            isPressed = false;
            return true;
        }

        if (hitTest(x, y)) {
            isPressed = true;
            isFocused = true;
            if (onClick) onClick();
            return true;
        } else {
            isFocused = false;
            isPressed = false;
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

#include <gui/GridLayout.hpp>
