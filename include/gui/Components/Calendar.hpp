#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <functional>
#include <ctime>

namespace MochiUI {

class Calendar : public FlexNode {
public:
    Calendar();
    
    void setDate(int year, int month, int day);
    void draw(SkCanvas* canvas) override;
    void syncSubtreeStyles() override;
    
    std::function<void(int, int, int)> onDateSelected;

private:
    int currentYear, currentMonth, currentDay;
    int displayedYear, displayedMonth;
    int selectedYear, selectedMonth, selectedDay;
    
    void updateGrid();
    void nextMonth();
    void prevMonth();
    
    bool needsUpdate = true;
    
    void onMouseEnter() override { isHovered = true; }
    void onMouseLeave() override { isHovered = false; }
};

} // namespace MochiUI
