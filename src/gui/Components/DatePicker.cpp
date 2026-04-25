#include <include/gui/Components/DatePicker.hpp>
#include <include/gui/Components/Calendar.hpp>
#include <include/gui/Components/Popover.hpp>
#include <include/gui/Components/TextNode.hpp>
#include <include/gui/Components/IconNode.hpp>
#include <include/gui/OverlayNode.hpp>
#include <include/core/Window.hpp>
#include <iomanip>
#include <sstream>

namespace MochiUI {

DatePicker::DatePicker() {
    time_t t = time(nullptr);
    struct tm* now = localtime(&t);
    selectedYear = now->tm_year + 1900;
    selectedMonth = now->tm_mon + 1;
    selectedDay = now->tm_mday;

    style.setWidth(200.0f);
    style.setHeight(36.0f);
    style.backgroundColor = Theme::Card;
    style.borderRadius = 6.0f;
    style.setPadding(8.0f);
    style.setFlexDirection(YGFlexDirectionRow);
    style.setAlignItems(YGAlignCenter);
    style.setGap(8.0f);
    enableHover = true;

    onClick = [this]() { showCalendar(); };
    
    updateText();
}

void DatePicker::updateText() {
    removeAllChildren();
    
    auto icon = std::make_shared<IconNode>();
    icon->setIcon("res://calendar.svg");
    icon->style.setWidth(18.0f);
    icon->style.setHeight(18.0f);
    icon->color = Theme::TextSecondary;
    addChild(icon);

    auto textNode = std::make_shared<TextNode>();
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << selectedDay << "/"
       << std::setfill('0') << std::setw(2) << selectedMonth << "/"
       << selectedYear;
    textNode->text = ss.str();
    textNode->fontSize = 14.0f;
    textNode->color = Theme::TextPrimary;
    textNode->style.setFlex(1.0f);
    addChild(textNode);
}

void DatePicker::setDate(int year, int month, int day) {
    selectedYear = year;
    selectedMonth = month;
    selectedDay = day;
    updateText();
}

void DatePicker::showCalendar() {
    auto calendar = std::make_shared<Calendar>();
    calendar->setDate(selectedYear, selectedMonth, selectedDay);
    
    // Find OverlayNode by climbing up
    FlexNode* current = this;
    OverlayNode* overlayNode = nullptr;
    while (current) {
        overlayNode = dynamic_cast<OverlayNode*>(current);
        if (overlayNode) break;
        current = current->parent;
    }

    if (!overlayNode) return;

    // Use absolute position for the popover anchor
    SkRect absoluteFrame = frame;
    auto popover = std::make_shared<Popover>(calendar, absoluteFrame);
    
    calendar->onDateSelected = [this, popover, overlayNode](int y, int m, int d) {
        setDate(y, m, d);
        if (onDateChanged) onDateChanged(y, m, d);
        overlayNode->removeOverlay(popover);
    };

    popover->onClose = [popover, overlayNode]() {
        overlayNode->removeOverlay(popover);
    };

    overlayNode->addOverlay(popover);
}

void DatePicker::draw(SkCanvas* canvas) {
    drawSelf(canvas);
    drawChildren(canvas);
}

Size DatePicker::measure(Size available) {
    return { 200.0f, 36.0f };
}

} // namespace MochiUI
