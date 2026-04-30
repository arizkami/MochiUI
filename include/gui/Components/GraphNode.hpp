#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <include/core/SkPath.h>
#include <include/core/SkPathBuilder.h>
#include <vector>
#include <deque>

namespace MochiUI {

class GraphNode : public FlexNode {
public:
    GraphNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }
    std::deque<float> dataPoints;
    size_t maxPoints = 50;
    SkColor lineColor = Theme::Accent;
    SkColor fillColor = SkColorSetA(Theme::Accent, 40);
    float strokeWidth = 2.0f;
    bool showGrid = true;

    void addValue(float val) {
        dataPoints.push_back(val);
        if (dataPoints.size() > maxPoints) {
            dataPoints.pop_front();
        }
    }

    Size measure(Size available) override {
        float w = (style.widthMode == SizingMode::Fixed) ? style.width : available.width;
        float h = (style.heightMode == SizingMode::Fixed) ? style.height : 150.0f;
        return { w, h };
    }

    void draw(SkCanvas* canvas) override {
        FlexNode::draw(canvas);
        if (dataPoints.empty()) return;

        float w = frame.width();
        float h = frame.height();
        float xStep = w / (maxPoints - 1);

        SkPathBuilder pathBuilder;
        SkPathBuilder areaPathBuilder;

        for (size_t i = 0; i < dataPoints.size(); ++i) {
            float x = frame.left() + i * xStep;
            float y = frame.bottom() - (dataPoints[i] * h);
            
            if (i == 0) {
                pathBuilder.moveTo(x, y);
                areaPathBuilder.moveTo(x, frame.bottom());
                areaPathBuilder.lineTo(x, y);
            } else {
                pathBuilder.lineTo(x, y);
                areaPathBuilder.lineTo(x, y);
            }
            
            if (i == dataPoints.size() - 1) {
                areaPathBuilder.lineTo(x, frame.bottom());
                areaPathBuilder.close();
            }
        }

        SkPaint fillPaint;
        fillPaint.setAntiAlias(true);
        fillPaint.setColor(fillColor);
        fillPaint.setStyle(SkPaint::kFill_Style);
        canvas->drawPath(areaPathBuilder.detach(), fillPaint);

        SkPaint linePaint;
        linePaint.setAntiAlias(true);
        linePaint.setColor(lineColor);
        linePaint.setStyle(SkPaint::kStroke_Style);
        linePaint.setStrokeWidth(strokeWidth);
        canvas->drawPath(pathBuilder.detach(), linePaint);
    }
};

} // namespace MochiUI
