#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>

namespace MochiUI {

class ToolbarNode : public FlexNode {
public:
    ToolbarNode() {
        style.setHeight(Theme::ControlHeight + 10.0f);
        style.setFlexDirection(YGFlexDirectionRow);
        style.setAlignItems(YGAlignCenter);
        style.setPadding(8, 0);
        style.setGap(8);
        style.backgroundColor = Theme::MenuBar;
        // Could add a bottom border manually in draw()
    }
};

} // namespace MochiUI
