#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>
#include <functional>
#include <optional>

namespace SphereUI {

struct SwitchNodeStyle {
    std::optional<SPHXColor> activeColor;
    std::optional<SPHXColor> inactiveColor;
    std::optional<SPHXColor> thumbColor;
    std::optional<SPHXColor> labelColor;
    std::optional<SPHXColor> borderColor;
    std::optional<SPHXColor> shadowColor;
    std::optional<float> fontSize;
    std::optional<float> switchWidth;
    std::optional<float> switchHeight;
    std::optional<float> spacing;
    std::optional<float> thumbInset;
    std::optional<float> borderWidth;
};

class SwitchNode : public FlexNode {
public:
    SwitchNode() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
    }

    std::string label;
    bool isOn = false;
    SPHXColor activeColor = SPHXColor(Theme::Accent);
    SPHXColor inactiveColor = SPHXColor(Theme::Border).withAlpha(uint8_t{180});
    SPHXColor thumbColor = SPHXColor(Theme::Card).lighter(0.18f);
    SPHXColor labelColor = SPHXColor(Theme::TextPrimary);
    float fontSize = 14.0f;
    float switchWidth = 58.0f;
    float switchHeight = 30.0f;
    float spacing = 10.0f;

    SwitchNodeStyle visualStyle;

    void setStyleOverrides(const SwitchNodeStyle& styleOverrides) { visualStyle = styleOverrides; }
    void clearStyleOverrides() { visualStyle = {}; }

    std::function<void(bool)> onChanged;

    Size measure(Size available) override;
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;

private:
    SwitchNodeStyle resolveStyle() const;
};

} // namespace SphereUI
