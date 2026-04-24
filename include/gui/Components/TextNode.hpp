#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <include/core/SkFont.h>
#include <include/core/SkTypeface.h>
#include <include/core/SkFontMgr.h>
#include <string>

namespace MochiUI {

class TextNode : public FlexNode {
public:
    TextNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }
    std::string text;
    SkColor color = SK_ColorBLACK;
    float fontSize = 16.0f;
    bool enableHover = false;  // Disable hover by default for text

    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
};

} // namespace MochiUI
