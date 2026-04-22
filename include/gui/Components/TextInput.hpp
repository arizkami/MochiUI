#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <string>
#include <functional>

namespace MochiUI {

class TextInput : public FlexNode {
public:
    std::string text;
    std::string placeholder;
    float fontSize = 14.0f;
    std::function<void(const std::string&)> onChanged;
    std::function<void()> onEnter;

    SkColor backgroundColor = Theme::Card;
    SkColor borderColor = SkColorSetA(Theme::TextSecondary, 50);
    SkColor focusColor = Theme::Accent;
    SkColor textColor = Theme::TextPrimary;
    SkColor placeholderColor = SkColorSetA(Theme::TextSecondary, 120);

    TextInput();
    
    void draw(SkCanvas* canvas) override;
    Size measure(Size available) override;
    
    bool onMouseDown(float x, float y) override;
    bool onChar(uint32_t charCode) override;
    bool onKeyDown(uint32_t key) override;
    bool needsRedraw() override;

private:
    size_t cursorIndex = 0;
    uint32_t lastBlinkTime = 0;
    bool showCursor = true;
};

} // namespace MochiUI
