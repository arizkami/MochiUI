#include <include/gui/Components/Calendar.hpp>
#include <include/gui/Components/TextNode.hpp>
#include <iomanip>
#include <sstream>

namespace MochiUI {

Calendar::Calendar() {
    time_t t = time(nullptr);
    struct tm* now = localtime(&t);
    currentYear = now->tm_year + 1900;
    currentMonth = now->tm_mon + 1;
    currentDay = now->tm_mday;
    
    selectedYear = currentYear;
    selectedMonth = currentMonth;
    selectedDay = currentDay;
    
    style.setWidth(280.0f);
    style.setHeight(300.0f);
    style.backgroundColor = Theme::Card;
    style.borderRadius = 8.0f;
    style.setPadding(10.0f);
    
    updateGrid();
}

void Calendar::setDate(int year, int month, int day) {
    selectedYear = year;
    selectedMonth = month;
    selectedDay = day;
    updateGrid();
}

void Calendar::nextMonth() {
    selectedMonth++;
    if (selectedMonth > 12) {
        selectedMonth = 1;
        selectedYear++;
    }
    updateGrid();
}

void Calendar::prevMonth() {
    selectedMonth--;
    if (selectedMonth < 1) {
        selectedMonth = 12;
        selectedYear--;
    }
    updateGrid();
}

void Calendar::updateGrid() {
    removeAllChildren();
    
    // Header with Month Year and Navigation
    auto header = FlexNode::Row();
    header->style.setHeight(40.0f);
    header->style.setAlignItems(YGAlignCenter);
    header->style.backgroundColor = SkColorSetA(Theme::Accent, 40);
    header->style.borderRadius = 6.0f;
    header->style.setMargin(2.0f);
    
    auto prevBtn = std::make_shared<TextNode>();
    prevBtn->text = " < ";
    prevBtn->fontSize = 18.0f;
    prevBtn->fontFamily = fontFamily;
    prevBtn->style.setPadding(5.0f);
    prevBtn->enableHover = true;
    prevBtn->onClick = [this]() { prevMonth(); };
    header->addChild(prevBtn);

    auto monthText = std::make_shared<TextNode>();
    std::string months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    monthText->text = months[selectedMonth-1] + " " + std::to_string(selectedYear);
    monthText->style.setFlex(1.0f);
    monthText->fontSize = 15.0f;
    monthText->fontFamily = fontFamily;
    monthText->style.setAlignItems(YGAlignCenter);
    header->addChild(monthText);

    auto nextBtn = std::make_shared<TextNode>();
    nextBtn->text = " > ";
    nextBtn->fontSize = 18.0f;
    nextBtn->fontFamily = fontFamily;
    nextBtn->style.setPadding(5.0f);
    nextBtn->enableHover = true;
    nextBtn->onClick = [this]() { nextMonth(); };
    header->addChild(nextBtn);

    addChild(header);

    // Weekday headers
    auto weekHeader = FlexNode::Row();
    weekHeader->style.setHeight(30.0f);
    std::string days_of_week[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
    for (const auto& d_name : days_of_week) {
        auto t = std::make_shared<TextNode>();
        t->text = d_name;
        t->fontSize = 12.0f;
        t->fontFamily = fontFamily;
        t->color = Theme::TextSecondary;
        t->style.setFlex(1.0f);
        weekHeader->addChild(t);
    }
    addChild(weekHeader);

    // Grid of days
    struct tm firstDay = {0};
    firstDay.tm_year = selectedYear - 1900;
    firstDay.tm_mon = selectedMonth - 1;
    firstDay.tm_mday = 1;
    mktime(&firstDay);
    
    int startDay = firstDay.tm_wday;
    int daysInMonth = 31;
    if (selectedMonth == 4 || selectedMonth == 6 || selectedMonth == 9 || selectedMonth == 11) daysInMonth = 30;
    else if (selectedMonth == 2) {
        bool leap = (selectedYear % 4 == 0 && selectedYear % 100 != 0) || (selectedYear % 400 == 0);
        daysInMonth = leap ? 29 : 28;
    }

    auto grid = FlexNode::Column();
    grid->style.setFlex(1.0f);
    
    auto row = FlexNode::Row();
    row->style.setHeight(35.0f);
    
    for (int i = 0; i < startDay; ++i) {
        auto empty = FlexNode::Create();
        empty->style.setFlex(1.0f);
        row->addChild(empty);
    }

    for (int d = 1; d <= daysInMonth; ++d) {
        if (row->children.size() == 7) {
            grid->addChild(row);
            row = FlexNode::Row();
            row->style.setHeight(35.0f);
        }
        
        auto dayNode = std::make_shared<TextNode>();
        dayNode->text = std::to_string(d);
        dayNode->fontSize = 13.0f;
        dayNode->fontFamily = fontFamily;
        dayNode->style.setFlex(1.0f);
        dayNode->style.borderRadius = 17.0f;
        dayNode->enableHover = true;
        
        if (d == selectedDay) {
            dayNode->style.backgroundColor = Theme::Accent;
            dayNode->color = SK_ColorWHITE;
        } else if (d == currentDay && selectedMonth == currentMonth && selectedYear == currentYear) {
            dayNode->color = Theme::Accent;
        }

        dayNode->onClick = [this, d]() {
            selectedDay = d;
            updateGrid();
            if (onDateSelected) onDateSelected(selectedYear, selectedMonth, selectedDay);
        };
        
        row->addChild(dayNode);
    }
    
    while (row->children.size() < 7) {
        auto empty = FlexNode::Create();
        empty->style.setFlex(1.0f);
        row->addChild(empty);
    }
    grid->addChild(row);
    addChild(grid);
}

void Calendar::draw(SkCanvas* canvas) {
    drawSelf(canvas);
    drawChildren(canvas);
}

Size Calendar::measure(Size available) {
    return { 280.0f, 300.0f };
}

} // namespace MochiUI
