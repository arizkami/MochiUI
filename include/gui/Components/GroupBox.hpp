#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>

namespace SphereUI {

class GroupBox : public FlexNode {
public:
    std::string title;
    float fontSize = 14.0f;
    SPHXColor borderColor = SPHXColor(Theme::TextSecondary).withAlpha(uint8_t(80));
    SPHXColor titleColor = Theme::TextSecondary;
    float strokeWidth = 1.0f;

    GroupBox();

    void draw(SkCanvas* canvas) override;
    void drawSelf(SkCanvas* canvas) override;
    void syncSubtreeStyles() override;
};

} // namespace SphereUI
