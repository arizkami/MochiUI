#include <gui/Components/Calendar.hpp>
#include <gui/Components/TextNode.hpp>
#include <gui/Components/IconNode.hpp>
#include <iomanip>
#include <sstream>

namespace MochiUI {

Calendar::Calendar() {
    time_t t = time(nullptr);
    struct tm* now = localtime(&t);
    currentYear = now->tm_year + 1900;
    currentMonth = now->tm_mon + 1;
    currentDay = now->tm_mday;
    
    displayedYear = currentYear;
    displayedMonth = currentMonth;
    
    selectedYear = currentYear;
    selectedMonth = currentMonth;
    selectedDay = currentDay;
    
    style.setWidth(280.0f);
    style.setHeightAuto();
    style.backgroundColor = Theme::Card;
    style.borderRadius = 12.0f;
    style.setPadding(16.0f);
    style.setGap(12.0f);
}

void Calendar::setDate(int year, int month, int day) {
    selectedYear = year;
    selectedMonth = month;
    selectedDay = day;
    displayedYear = year;
    displayedMonth = month;
    needsUpdate = true;
    markDirty();
}

void Calendar::nextMonth() {
    displayedMonth++;
    if (displayedMonth > 12) {
        displayedMonth = 1;
        displayedYear++;
    }
    needsUpdate = true;
    markDirty();
}

void Calendar::prevMonth() {
    displayedMonth--;
    if (displayedMonth < 1) {
        displayedMonth = 12;
        displayedYear--;
    }
    needsUpdate = true;
    markDirty();
}

void Calendar::syncSubtreeStyles() {
    if (needsUpdate) {
        updateGrid();
    }
    FlexNode::syncSubtreeStyles();
}

void Calendar::updateGrid() {
    needsUpdate = false;
    removeAllChildren();
    
    // Header
    auto header = FlexNode::Row();
    header->style.setHeight(32.0f);
    header->style.setAlignItems(YGAlignCenter);
    
    auto monthYearText = std::make_shared<TextNode>();
    static const std::string months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    monthYearText->text = months[displayedMonth-1] + " " + std::to_string(displayedYear);
    monthYearText->style.setFlex(1.0f);
    monthYearText->fontSize = 16.0f;
    monthYearText->color = Theme::TextPrimary;
    header->addChild(monthYearText);

    auto navGroup = FlexNode::Row();
    navGroup->style.setGap(8.0f);

    auto prevBtn = std::make_shared<IconNode>();
    prevBtn->setIcon("res://chevron-left.svg");
    prevBtn->style.setWidth(24.0f);
    prevBtn->style.setHeight(24.0f);
    prevBtn->color = Theme::TextSecondary;
    prevBtn->enableHover = true;
    prevBtn->onClick = [this]() { prevMonth(); };
    navGroup->addChild(prevBtn);

    auto nextBtn = std::make_shared<IconNode>();
    nextBtn->setIcon("res://chevron-right.svg");
    nextBtn->style.setWidth(24.0f);
    nextBtn->style.setHeight(24.0f);
    nextBtn->color = Theme::TextSecondary;
    nextBtn->enableHover = true;
    nextBtn->onClick = [this]() { nextMonth(); };
    navGroup->addChild(nextBtn);

    header->addChild(navGroup);
    addChild(header);

    // Weekdays
    auto weekHeader = FlexNode::Row();
    weekHeader->style.setHeight(24.0f);
    static const std::string days_of_week[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
    for (const auto& d_name : days_of_week) {
        auto t = std::make_shared<TextNode>();
        t->text = d_name;
        t->fontSize = 12.0f;
        t->color = Theme::TextSecondary;
        t->textAlign = TextAlign::Center;
        t->style.setFlex(1.0f);
        weekHeader->addChild(t);
    }
    addChild(weekHeader);

    // Days Grid
    struct tm firstDay = {0};
    firstDay.tm_year = displayedYear - 1900;
    firstDay.tm_mon = displayedMonth - 1;
    firstDay.tm_mday = 1;
    mktime(&firstDay);
    
    int startDay = firstDay.tm_wday;
    int daysInMonth = 31;
    if (displayedMonth == 4 || displayedMonth == 6 || displayedMonth == 9 || displayedMonth == 11) daysInMonth = 30;
    else if (displayedMonth == 2) {
        bool leap = (displayedYear % 4 == 0 && displayedYear % 100 != 0) || (displayedYear % 400 == 0);
        daysInMonth = leap ? 29 : 28;
    }

    auto grid = FlexNode::Column();
    grid->style.setGap(4.0f);
    
    auto row = FlexNode::Row();
    row->style.setHeight(32.0f);
    row->style.setGap(4.0f);
    
    // Padding for start of month
    for (int i = 0; i < startDay; ++i) {
        auto empty = FlexNode::Create();
        empty->style.setFlex(1.0f);
        row->addChild(empty);
    }

    for (int d = 1; d <= daysInMonth; ++d) {
        if (row->children.size() == 7) {
            grid->addChild(row);
            row = FlexNode::Row();
            row->style.setHeight(32.0f);
            row->style.setGap(4.0f);
        }
        
        auto dayNode = std::make_shared<TextNode>();
        dayNode->text = std::to_string(d);
        dayNode->fontSize = 14.0f;
        dayNode->textAlign = TextAlign::Center;
        dayNode->style.setFlex(1.0f);
        dayNode->style.setHeight(32.0f);
        dayNode->style.borderRadius = 16.0f;
        dayNode->enableHover = true;
        
        bool isSelected = (d == selectedDay && displayedMonth == selectedMonth && displayedYear == selectedYear);
        bool isToday = (d == currentDay && displayedMonth == currentMonth && displayedYear == currentYear);

        if (isSelected) {
            dayNode->style.backgroundColor = Theme::Accent;
            dayNode->color = SK_ColorWHITE;
        } else if (isToday) {
            dayNode->color = Theme::Accent;
            dayNode->style.backgroundColor = SkColorSetA(Theme::Accent, 40);
        } else {
            dayNode->color = Theme::TextPrimary;
        }

        dayNode->onClick = [this, d]() {
            selectedDay = d;
            selectedMonth = displayedMonth;
            selectedYear = displayedYear;
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

} // namespace MochiUI
