#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>
#include <functional>

namespace SphereUI {

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

    SPHXColor backgroundColor  = SPHXColor::white();
    SPHXColor borderColor      = SPHXColor::RGB(169, 169, 169);   // 1px solid gray border
    SPHXColor focusColor       = SPHXColor::RGB(0, 120, 215);     // blue focus ring
    SPHXColor textColor        = SPHXColor::black();
    SPHXColor placeholderColor = SPHXColor::RGB(128, 128, 128);

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

} // namespace SphereUI
