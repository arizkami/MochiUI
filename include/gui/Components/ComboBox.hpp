#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <string>
#include <vector>
#include <functional>

namespace MochiUI {

class ComboBox : public FlexNode {
public:
    ComboBox() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }
    std::vector<std::string> items;
    int selectedIndex = -1;
    std::string placeholder = "Select item...";
    std::function<void(int)> onSelectionChanged;

    SkColor backgroundColor = Theme::Card;
    SkColor borderColor = SkColorSetA(Theme::TextSecondary, 50);
    SkColor textColor = Theme::TextPrimary;
    SkColor accentColor = Theme::Accent;
    float fontSize = Theme::FontNormal;

    void draw(SkCanvas* canvas) override;
    Size measure(Size available) override;
    
    bool onMouseDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;

private:
    bool isOpen = false;
    int hoveredItemIndex = -1;
    float itemHeight = Theme::ControlHeight;
    
    SkRect getDropdownRect() const;
};

} // namespace MochiUI
