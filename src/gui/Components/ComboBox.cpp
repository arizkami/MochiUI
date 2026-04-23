#include <include/gui/Components/ComboBox.hpp>
#include <include/core/SkPathBuilder.h>

namespace MochiUI {

ComboBox::ComboBox() {
    style.widthMode = SizingMode::Flex;
    style.heightMode = SizingMode::Fixed;
    style.height = 28.0f;
    style.padding = 4.0f;
    style.borderRadius = 0.0f;
}

Size ComboBox::measure(Size available) {
    float w = (style.widthMode == SizingMode::Fixed) ? style.width : 120.0f;
    float h = (style.heightMode == SizingMode::Fixed) ? style.height : 28.0f;
    return { w, h };
}

SkRect ComboBox::getDropdownRect() const {
    float h = items.size() * itemHeight;
    return SkRect::MakeXYWH(frame.left(), frame.bottom(), frame.width(), h);
}

void ComboBox::draw(SkCanvas* canvas) {
    // 1. Draw Main Box
    SkPaint bgPaint;
    bgPaint.setAntiAlias(true);
    bgPaint.setColor(backgroundColor);
    canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, bgPaint);

    SkPaint borderPaint;
    borderPaint.setAntiAlias(true);
    borderPaint.setStyle(SkPaint::kStroke_Style);
    borderPaint.setStrokeWidth(1.0f);
    borderPaint.setColor(isOpen ? accentColor : borderColor);
    canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, borderPaint);

    // 2. Draw Current Selection
    SkPaint textPaint;
    textPaint.setAntiAlias(true);
    textPaint.setColor(selectedIndex >= 0 ? textColor : SkColorSetA(textColor, 120));

    std::string currentText = (selectedIndex >= 0 && (size_t)selectedIndex < items.size()) 
                              ? items[selectedIndex] : placeholder;
    
    FontManager::getInstance().drawText(canvas, currentText, 
                                        frame.left() + style.padding, frame.centerY() + 5.0f, 
                                        14.0f, textPaint);

    // 3. Draw Arrow
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

    // 4. Draw Dropdown if open
    if (isOpen) {
        SkRect dropdownRect = getDropdownRect();
        
        // Dropdown background
        SkPaint dropdownBgPaint;
        dropdownBgPaint.setAntiAlias(true);
        dropdownBgPaint.setColor(backgroundColor);
        canvas->drawRect(dropdownRect, dropdownBgPaint);

        // Border for dropdown
        canvas->drawRect(dropdownRect, borderPaint);

        for (size_t i = 0; i < items.size(); ++i) {
            SkRect itemRect = SkRect::MakeXYWH(dropdownRect.left(), dropdownRect.top() + i * itemHeight, 
                                              dropdownRect.width(), itemHeight);
            
            // Hover effect
            if ((int)i == hoveredItemIndex) {
                SkPaint hoverPaint;
                hoverPaint.setColor(accentColor);
                hoverPaint.setAlphaf(0.2f);
                canvas->drawRect(itemRect, hoverPaint);
            }

            // Item text
            textPaint.setColor(textColor);
            FontManager::getInstance().drawText(canvas, items[i], 
                                                itemRect.left() + style.padding, itemRect.centerY() + 5.0f, 
                                                14.0f, textPaint);
        }
    }
}

bool ComboBox::onMouseDown(float x, float y) {
    if (frame.contains(x, y)) {
        isOpen = !isOpen;
        return true;
    }

    if (isOpen) {
        SkRect dropdownRect = getDropdownRect();
        if (dropdownRect.contains(x, y)) {
            int index = (int)((y - dropdownRect.top()) / itemHeight);
            if (index >= 0 && (size_t)index < items.size()) {
                selectedIndex = index;
                if (onSelectionChanged) onSelectionChanged(selectedIndex);
            }
            isOpen = false;
            return true;
        }
    }

    if (isOpen) {
        isOpen = false;
        return true;
    }

    return false;
}

bool ComboBox::onMouseMove(float x, float y) {
    bool handled = FlexNode::onMouseMove(x, y);
    
    if (isOpen) {
        SkRect dropdownRect = getDropdownRect();
        if (dropdownRect.contains(x, y)) {
            hoveredItemIndex = (int)((y - dropdownRect.top()) / itemHeight);
            handled = true;
        } else {
            hoveredItemIndex = -1;
        }
    }
    
    return handled || isOpen;
}

} // namespace MochiUI
