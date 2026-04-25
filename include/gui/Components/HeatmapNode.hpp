#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>

namespace MochiUI {

class HeatmapNode : public FlexNode {
public:
    HeatmapNode(int rows, int cols) : rows(rows), cols(cols) {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        style.setFlexDirection(YGFlexDirectionColumn);
        style.setGap(2);
        
        for (int r = 0; r < rows; ++r) {
            auto rowNode = FlexNode::Row();
            rowNode->style.setGap(2);
            for (int c = 0; c < cols; ++c) {
                auto cell = FlexNode::Create();
                cell->style.setWidth(15);
                cell->style.setHeight(15);
                cell->style.backgroundColor = SkColorSetRGB(230, 230, 230);
                rowNode->addChild(cell);
            }
            addChild(rowNode);
        }
    }

    Size measure(Size available) override {
        float w = cols * 15.0f + (cols - 1) * 2.0f;
        float h = rows * 15.0f + (rows - 1) * 2.0f;
        return { w, h };
    }

private:
    int rows, cols;
};

} // namespace MochiUI
