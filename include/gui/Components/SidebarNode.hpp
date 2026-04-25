#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>

namespace MochiUI {

class SidebarNode : public FlexNode {
public:
    SidebarNode() {
        style.setWidth(250);
        style.setHeightFull();
        style.backgroundColor = Theme::Sidebar;
        style.setPadding(10);
        style.setGap(5);
    }

    void setCollapsed(bool collapsed) {
        style.setWidth(collapsed ? 60 : 250);
    }
};

} // namespace MochiUI
