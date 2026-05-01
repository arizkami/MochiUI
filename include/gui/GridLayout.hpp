#pragma once
#include <algorithm>
#include <memory>
#include <gui/Layout.hpp>

namespace AureliaUI {

// Equal-size cell grid: children are laid out row-major in columnCount columns.
// Uses style.gap between cells and respects padding on the grid node.
//
// Grid placement runs in applyYogaLayout (not calculateLayout) so nested grids
// work when the window only calls calculateLayout on the root FlexNode.
class GridLayoutNode : public FlexNode {
public:
    using Ptr = std::shared_ptr<GridLayoutNode>;

    int columnCount = 1;

    GridLayoutNode() {
        style.setWidthFull();
        style.setHeightFull();
    }

    void setColumns(int n) {
        columnCount = std::max(1, n);
        markDirty();
    }

    void applyYogaLayout(float offsetX, float offsetY) override {
        float left = YGNodeLayoutGetLeft(getYGNode()) + offsetX;
        float top = YGNodeLayoutGetTop(getYGNode()) + offsetY;
        float width = YGNodeLayoutGetWidth(getYGNode());
        float height = YGNodeLayoutGetHeight(getYGNode());
        frame = SkRect::MakeXYWH(left, top, width, height);

        const int cols = std::max(1, columnCount);
        const int n = static_cast<int>(children.size());
        if (n == 0) {
            return;
        }

        const float gap = style.gap;
        const float padL = getLayoutPadding(YGEdgeLeft);
        const float padT = getLayoutPadding(YGEdgeTop);
        const float padR = getLayoutPadding(YGEdgeRight);
        const float padB = getLayoutPadding(YGEdgeBottom);

        const float innerLeft = frame.left() + padL;
        const float innerTop = frame.top() + padT;
        const float innerW = std::max(0.0f, frame.width() - padL - padR);
        const float innerH = std::max(0.0f, frame.height() - padT - padB);

        const int rows = (n + cols - 1) / cols;
        const float totalGapW = gap * static_cast<float>(std::max(0, cols - 1));
        const float totalGapH = gap * static_cast<float>(std::max(0, rows - 1));
        const float cellW = cols > 0 ? (innerW - totalGapW) / static_cast<float>(cols) : innerW;
        const float cellH = rows > 0 ? (innerH - totalGapH) / static_cast<float>(rows) : innerH;

        for (int i = 0; i < n; ++i) {
            const int col = i % cols;
            const int row = i / cols;
            const float cellLeft = innerLeft + static_cast<float>(col) * (cellW + gap);
            const float cellTop = innerTop + static_cast<float>(row) * (cellH + gap);

            FlexNode::Ptr& c = children[static_cast<size_t>(i)];
            c->syncSubtreeStyles();
            YGNodeCalculateLayout(c->getYGNode(), std::max(0.0f, cellW), std::max(0.0f, cellH), YGDirectionLTR);
            c->applyYogaLayout(cellLeft, cellTop);
        }
    }

    static Ptr Create() { return std::make_shared<GridLayoutNode>(); }
};

} // namespace AureliaUI
