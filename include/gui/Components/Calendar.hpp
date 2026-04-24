#pragma once
#include <include/gui/Layout.hpp>
#include <include/gui/Theme.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <functional>
#include <ctime>

namespace MochiUI {

class Calendar : public FlexNode {
public:
    Calendar();
    
    void setDate(int year, int month, int day);
    void draw(SkCanvas* canvas) override;
    Size measure(Size available) override;
    std::string fontFamily = FontManager::DEFAULT_FONT;
    
    std::function<void(int, int, int)> onDateSelected;

private:
    int currentYear, currentMonth, currentDay;
    int selectedYear, selectedMonth, selectedDay;
    
    void updateGrid();
    void nextMonth();
    void prevMonth();
    
    struct DayInfo {
        int day;
        bool currentMonth;
        bool isSelected;
        bool isToday;
        SkRect rect;
    };
    std::vector<DayInfo> days;
};

} // namespace MochiUI
