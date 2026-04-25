#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <string>
#include <functional>

namespace MochiUI {

class TextInput : public FlexNode {
public:
    TextInput() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
        enableHover = true;
        style.cursorType = Cursor::IBeam;
    }
    std::string text;
    std::string placeholder;
    float fontSize = Theme::FontNormal;
    std::function<void(const std::string&)> onChanged;
    std::function<void()> onEnter;

    SkColor backgroundColor = Theme::Card;
    SkColor borderColor = SkColorSetA(Theme::TextSecondary, 50);
    SkColor focusColor = Theme::Accent;
    SkColor textColor = Theme::TextPrimary;
    SkColor placeholderColor = SkColorSetA(Theme::TextSecondary, 120);

    void draw(SkCanvas* canvas) override;
    Size measure(Size available) override;
    
    bool onMouseDown(float x, float y) override;
    bool onRightDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;
    void onMouseUp(float x, float y) override;
    bool onChar(uint32_t charCode) override;
    bool onKeyDown(uint32_t key) override;
    bool needsRedraw() override;

private:
    size_t cursorIndex = 0;
    size_t selectionAnchor = std::string::npos;
    bool isDragging = false;
    uint32_t lastBlinkTime = 0;
    bool showCursor = true;
    uint16_t highSurrogate = 0;
    
    void deleteSelection();
    bool hasSelection() const;
    size_t getSelectionStart() const;
    size_t getSelectionEnd() const;
    size_t getCursorIndexFromPosition(float x);
};

} // namespace MochiUI
