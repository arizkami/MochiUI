#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <string>
#include <vector>
#include <functional>

namespace SphereUI {

// Overlay node that renders the open dropdown on top of all other content.
class ComboBoxDropdown : public FlexNode {
public:
    using Ptr = std::shared_ptr<ComboBoxDropdown>;

    SkRect dropdownRect = SkRect::MakeEmpty();
    const std::vector<std::string>* items = nullptr;
    SPHXColor backgroundColor = SPHXColor::white();       // solid; must be opaque
    SPHXColor borderColor     = SPHXColor::RGB(169, 169, 169);
    SPHXColor textColor       = SPHXColor::black();
    SPHXColor accentColor     = SPHXColor::RGB(0, 120, 215);
    float fontSize   = Theme::FontNormal;
    float itemHeight = Theme::ControlHeight;
    int hoveredItemIndex = -1;
    std::function<void(int)> onPick; // idx >= 0 = selection, -1 = dismiss

    bool hitTest(float x, float y) override { return true; } // absorb all events
    void draw(SkCanvas* canvas) override;
    bool onMouseDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;
};

class ComboBox : public FlexNode {
public:
    ComboBox() {
        YGNodeSetMeasureFunc(getYGNode(), &FlexNode::MeasureCallback);
    }

    ~ComboBox() {
        if (isOpen && windowHost && dropdownOverlay)
            windowHost->removeOverlay(dropdownOverlay);
    }

    std::vector<std::string> items;
    int selectedIndex = -1;
    std::string placeholder = "Select item...";
    std::function<void(int)> onSelectionChanged;

    void addItem(const std::string& item) { items.push_back(item); }

    SPHXColor backgroundColor = SPHXColor::white();
    SPHXColor borderColor     = SPHXColor::RGB(169, 169, 169);
    SPHXColor textColor       = SPHXColor::black();
    SPHXColor accentColor     = SPHXColor::RGB(0, 120, 215);
    float fontSize = Theme::FontNormal;

    void draw(SkCanvas* canvas) override;
    Size measure(Size available) override;
    bool onMouseDown(float x, float y) override;
    bool onMouseMove(float x, float y) override;

private:
    bool isOpen = false;
    float itemHeight = Theme::ControlHeight;
    ComboBoxDropdown::Ptr dropdownOverlay;

    SkRect getDropdownRect() const;
    void openDropdown();
    void closeDropdown();
};

} // namespace SphereUI
