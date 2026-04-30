#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkFont.h>
#include <include/core/SkFontMetrics.h>
#include <string>
#include <functional>

namespace MochiUI {

class CheckboxNode : public FlexNode {
public:
    CheckboxNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
    }
    std::string label;
    bool checked = false;
    SkColor checkboxColor = Theme::Accent;
    SkColor labelColor = Theme::TextPrimary;
    float fontSize = 14.0f;
    float checkboxSize = 18.0f;
    float spacing = 8.0f;
    std::function<void(bool)> onChanged;
    
    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;
};

} // namespace MochiUI
