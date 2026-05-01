#include <gui/Components/ComboBox.hpp>

namespace AureliaUI {

// ---- ComboBoxDropdown ----

void ComboBoxDropdown::draw(SkCanvas* canvas) {
    if (!items || items->empty()) return;

    // Shadow
    SkPaint shadowPaint;
    shadowPaint.setAntiAlias(true);
    shadowPaint.setColor(SkColorSetARGB(60, 0, 0, 0));
    SkRect shadowRect = dropdownRect.makeOffset(0, 3);
    canvas->drawRoundRect(shadowRect, 6, 6, shadowPaint);

    // Opaque background
    SkPaint bgPaint;
    bgPaint.setAntiAlias(true);
    bgPaint.setColor(backgroundColor);
    canvas->drawRoundRect(dropdownRect, 6, 6, bgPaint);

    // Border
    SkPaint borderPaint;
    borderPaint.setAntiAlias(true);
    borderPaint.setStyle(SkPaint::kStroke_Style);
    borderPaint.setStrokeWidth(1.0f);
    borderPaint.setColor(borderColor);
    canvas->drawRoundRect(dropdownRect, 6, 6, borderPaint);

    // Clip items to rounded rect so rows don't overflow corners
    canvas->save();
    canvas->clipRRect(SkRRect::MakeRectXY(dropdownRect, 6, 6), true);

    constexpr float hPad = 8.0f;
    SkPaint textPaint;
    textPaint.setAntiAlias(true);

    for (size_t i = 0; i < items->size(); ++i) {
        SkRect itemRect = SkRect::MakeXYWH(
            dropdownRect.left(),
            dropdownRect.top() + (float)i * itemHeight,
            dropdownRect.width(), itemHeight);

        if ((int)i == hoveredItemIndex) {
            SkPaint hoverPaint;
            hoverPaint.setAntiAlias(true);
            hoverPaint.setColor(accentColor);
            hoverPaint.setAlphaf(0.15f);
            canvas->drawRect(itemRect, hoverPaint);
        }

        textPaint.setColor(textColor);
        FontManager::getInstance().drawText(canvas, (*items)[i],
            itemRect.left() + hPad,
            itemRect.centerY() + fontSize / 2.0f - 2.0f,
            fontSize, textPaint);
    }

    canvas->restore();
}

bool ComboBoxDropdown::onMouseDown(float x, float y) {
    if (dropdownRect.contains(x, y) && items) {
        int idx = (int)((y - dropdownRect.top()) / itemHeight);
        if (idx >= 0 && (size_t)idx < items->size()) {
            if (onPick) onPick(idx);
            return true;
        }
    }
    // Click outside dropdown — dismiss
    if (onPick) onPick(-1);
    return true;
}

bool ComboBoxDropdown::onMouseMove(float x, float y) {
    int prev = hoveredItemIndex;
    if (dropdownRect.contains(x, y) && items) {
        hoveredItemIndex = (int)((y - dropdownRect.top()) / itemHeight);
        if (hoveredItemIndex < 0 || (size_t)hoveredItemIndex >= items->size())
            hoveredItemIndex = -1;
    } else {
        hoveredItemIndex = -1;
    }
    if (hoveredItemIndex != prev) requestRedraw();
    return true; // consume to block underlying hover
}

// ---- ComboBox ----

Size ComboBox::measure(Size available) {
    return { 150.0f, Theme::ControlHeight };
}

SkRect ComboBox::getDropdownRect() const {
    float h = (float)items.size() * itemHeight;
    return SkRect::MakeXYWH(frame.left(), frame.bottom(), frame.width(), h);
}

void ComboBox::openDropdown() {
    if (!windowHost || items.empty()) return;
    isOpen = true;

    dropdownOverlay = std::make_shared<ComboBoxDropdown>();
    // Absolute position so Yoga doesn't push it into the layout flow
    dropdownOverlay->style.setPositionType(YGPositionTypeAbsolute);
    dropdownOverlay->dropdownRect = getDropdownRect();
    dropdownOverlay->items = &items;
    // Use Theme::Sidebar as dropdown background — always fully opaque
    dropdownOverlay->backgroundColor = Theme::Sidebar;
    dropdownOverlay->borderColor = borderColor;
    dropdownOverlay->textColor = textColor;
    dropdownOverlay->accentColor = accentColor;
    dropdownOverlay->fontSize = fontSize;
    dropdownOverlay->itemHeight = itemHeight;
    dropdownOverlay->onPick = [this](int idx) {
        if (idx >= 0) {
            selectedIndex = idx;
            if (onSelectionChanged) onSelectionChanged(selectedIndex);
        }
        closeDropdown();
        requestRedraw();
    };

    windowHost->addOverlay(dropdownOverlay);
    requestRedraw();
}

void ComboBox::closeDropdown() {
    if (!isOpen) return;
    isOpen = false;
    if (windowHost && dropdownOverlay) windowHost->removeOverlay(dropdownOverlay);
    dropdownOverlay.reset();
}

void ComboBox::draw(SkCanvas* canvas) {
    float r = style.borderRadius > 0 ? style.borderRadius : Theme::BorderRadius;

    // Background — draw fully opaque by blending alpha into the fill
    SkPaint bgPaint;
    bgPaint.setAntiAlias(true);
    bgPaint.setColor(backgroundColor);
    canvas->drawRoundRect(frame, r, r, bgPaint);

    // Border (accent-colored when open)
    SkPaint borderPaint;
    borderPaint.setAntiAlias(true);
    borderPaint.setStyle(SkPaint::kStroke_Style);
    borderPaint.setStrokeWidth(1.0f);
    borderPaint.setColor(isOpen ? accentColor : borderColor);
    canvas->drawRoundRect(frame, r, r, borderPaint);

    // Text — 8px left padding keeps text away from the border
    constexpr float hPad = 8.0f;
    SkPaint textPaint;
    textPaint.setAntiAlias(true);
    textPaint.setColor(selectedIndex >= 0 ? textColor : textColor.withAlpha(uint8_t(120)));

    std::string currentText = (selectedIndex >= 0 && (size_t)selectedIndex < items.size())
                              ? items[selectedIndex] : placeholder;
    FontManager::getInstance().drawText(canvas, currentText,
        frame.left() + hPad,
        frame.centerY() + fontSize / 2.0f - 2.0f,
        fontSize, textPaint);

    // Chevron arrow
    SkPaint arrowPaint;
    arrowPaint.setAntiAlias(true);
    arrowPaint.setStyle(SkPaint::kStroke_Style);
    arrowPaint.setStrokeWidth(1.5f);
    arrowPaint.setColor(Theme::TextSecondary);

    float arrowSize = 4.0f;
    float arrowCenterX = frame.right() - 15.0f;
    float arrowCenterY = frame.centerY();

    SkPathBuilder arrowPath;
    if (isOpen) {
        arrowPath.moveTo(arrowCenterX - arrowSize, arrowCenterY + arrowSize / 2);
        arrowPath.lineTo(arrowCenterX, arrowCenterY - arrowSize / 2);
        arrowPath.lineTo(arrowCenterX + arrowSize, arrowCenterY + arrowSize / 2);
    } else {
        arrowPath.moveTo(arrowCenterX - arrowSize, arrowCenterY - arrowSize / 2);
        arrowPath.lineTo(arrowCenterX, arrowCenterY + arrowSize / 2);
        arrowPath.lineTo(arrowCenterX + arrowSize, arrowCenterY - arrowSize / 2);
    }
    canvas->drawPath(arrowPath.detach(), arrowPaint);

    // Keep dropdown position in sync if the layout shifts
    if (isOpen && dropdownOverlay)
        dropdownOverlay->dropdownRect = getDropdownRect();
}

bool ComboBox::onMouseDown(float x, float y) {
    if (frame.contains(x, y)) {
        if (isOpen) closeDropdown();
        else openDropdown();
        requestRedraw();
        return true;
    }
    return false;
}

bool ComboBox::onMouseMove(float x, float y) {
    return FlexNode::onMouseMove(x, y);
}

} // namespace AureliaUI
