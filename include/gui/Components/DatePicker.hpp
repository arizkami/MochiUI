#pragma once
#include <gui/Layout.hpp>
#include <gui/Theme.hpp>
#include <ctime>

namespace SphereUI {

class DatePicker : public FlexNode {
public:
    DatePicker();

    void setDate(int year, int month, int day);
    void draw(SkCanvas* canvas) override;
    Size measure(Size available) override;

    std::function<void(int, int, int)> onDateChanged;

private:
    int selectedYear, selectedMonth, selectedDay;
    void showCalendar();
    void updateText();
};

} // namespace SphereUI
