#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>

namespace SphereUI {

class SplitterNode : public FlexNode {
public:
    enum class Orientation { Horizontal, Vertical };

    SplitterNode(Orientation orientation = Orientation::Horizontal)
        : orientation(orientation) {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        style.cursorType = (orientation == Orientation::Horizontal) ? Cursor::SizeNS : Cursor::SizeWE;
        style.backgroundColor = Theme::Border;
        if (orientation == Orientation::Horizontal) {
            style.setHeight(4);
            style.setWidthFull();
        } else {
            style.setWidth(4);
            style.setHeightFull();
        }
        enableHover = true;
    }

    Orientation orientation;
    std::function<void(float)> onDrag;

    Size measure(Size available) override {
        if (orientation == Orientation::Horizontal) {
            float w = IsUndefined(available.width) ? 100.0f : available.width;
            return { w, 4.0f };
        } else {
            float h = IsUndefined(available.height) ? 100.0f : available.height;
            return { 4.0f, h };
        }
    }

    bool onMouseDown(float x, float y) override {
        if (hitTest(x, y)) {
            isDragging = true;
            lastPos = (orientation == Orientation::Horizontal) ? y : x;
            return true;
        }
        return false;
    }

    bool onMouseMove(float x, float y) override {
        bool handled = FlexNode::onMouseMove(x, y);
        if (isDragging) {
            float currentPos = (orientation == Orientation::Horizontal) ? y : x;
            float delta = currentPos - lastPos;
            if (onDrag) onDrag(delta);
            lastPos = currentPos;
            return true;
        }
        return handled;
    }

    void onMouseUp(float x, float y) override {
        isDragging = false;
        FlexNode::onMouseUp(x, y);
    }

private:
    bool isDragging = false;
    float lastPos = 0;
};

} // namespace SphereUI
