#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>

namespace SphereUI {

enum class TextAlign { Left, Center, Right };

class TextNode : public FlexNode {
public:
    TextNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }
    TextNode(std::string text) : text(text) {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }
    std::string text;
    std::string fontFamily = FontManager::DEFAULT_FONT;
    float fontSize = Theme::FontNormal;
    SPHXColor color = SPHXColor(Theme::TextPrimary);
    TextAlign textAlign = TextAlign::Left;
    // CSS-like font weight. Kept separate from fontBold so callers can request Medium/Semibold.
    int fontWeight = 400;
    bool fontBold = false;

    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
};

} // namespace SphereUI
