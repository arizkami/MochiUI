#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>

namespace MochiUI {

class GroupBox : public FlexNode {
public:
    std::string title;
    float fontSize = 14.0f;
    SkColor borderColor = SkColorSetA(Theme::TextSecondary, 80);
    SkColor titleColor = Theme::TextSecondary;
    float strokeWidth = 1.0f;

    GroupBox();
    
    void draw(SkCanvas* canvas) override;
    void drawSelf(SkCanvas* canvas) override;
    void syncSubtreeStyles() override;
};

} // namespace MochiUI
