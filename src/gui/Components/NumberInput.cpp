#include <gui/Components/NumberInput.hpp>
#include <windows.h>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace SphereUI {

NumberInput::NumberInput() {
    syncTextFromValue();
    style.setPadding(4.0f);
}

void NumberInput::syncTextFromValue() {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    std::string s = ss.str();

    // Remove trailing zeros
    if (precision > 0) {
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') s.pop_back();
    }

    text = s;
}

void NumberInput::syncValueFromText() {
    try {
        if (text.empty() || text == "-" || text == "." || text == "-." || text == "-0") {
            return;
        }
        double newVal = std::stod(text);
        newVal = std::clamp(newVal, min, max);
        if (std::abs(newVal - value) > 1e-9) {
            value = newVal;
            if (onValueChanged) onValueChanged(value);
        }
    } catch (...) {}
}

SkRect NumberInput::getUpButtonRect() const {
    float btnW = 24.0f;
    float btnH = frame.height() / 2.0f;
    return SkRect::MakeXYWH(frame.right() - btnW, frame.top(), btnW, btnH);
}

SkRect NumberInput::getDownButtonRect() const {
    float btnW = 24.0f;
    float btnH = frame.height() / 2.0f;
    return SkRect::MakeXYWH(frame.right() - btnW, frame.top() + btnH, btnW, btnH);
}

void NumberInput::draw(SkCanvas* canvas) {
    TextInput::draw(canvas); // TextInput now calls drawSelf and drawChildren

    // Draw spin buttons on top
    SkRect upRect = getUpButtonRect();
    SkRect downRect = getDownButtonRect();

    SkPaint btnPaint;
    btnPaint.setAntiAlias(true);
    btnPaint.setColor(SkColorSetA(Theme::TextSecondary, 40));

    canvas->drawRect(upRect, btnPaint);
    canvas->drawRect(downRect, btnPaint);

    SkPaint arrowPaint;
    arrowPaint.setAntiAlias(true);
    arrowPaint.setStyle(SkPaint::kStroke_Style);
    arrowPaint.setStrokeWidth(1.5f);
    arrowPaint.setColor(Theme::TextSecondary);

    // Up arrow
    SkPathBuilder upPath;
    upPath.moveTo(upRect.centerX() - 4, upRect.centerY() + 2);
    upPath.lineTo(upRect.centerX(), upRect.centerY() - 2);
    upPath.lineTo(upRect.centerX() + 4, upRect.centerY() + 2);
    canvas->drawPath(upPath.detach(), arrowPaint);

    // Down arrow
    SkPathBuilder downPath;
    downPath.moveTo(downRect.centerX() - 4, downRect.centerY() - 2);
    downPath.lineTo(downRect.centerX(), downRect.centerY() + 2);
    downPath.lineTo(downRect.centerX() + 4, downRect.centerY() - 2);
    canvas->drawPath(downPath.detach(), arrowPaint);
}

bool NumberInput::onChar(uint32_t charCode) {
    if (!isFocused) return false;

    // Only allow numbers, dot, and minus
    if ((charCode >= '0' && charCode <= '9') || charCode == '.' || charCode == '-' || charCode == 8) {
        bool handled = TextInput::onChar(charCode);
        if (handled) syncValueFromText();
        return handled;
    }
    return true;
}

bool NumberInput::onKeyDown(uint32_t key) {
    if (isFocused) {
        if (key == VK_UP) {
            value = std::clamp(value + step, min, max);
            syncTextFromValue();
            if (onValueChanged) onValueChanged(value);
            return true;
        } else if (key == VK_DOWN) {
            value = std::clamp(value - step, min, max);
            syncTextFromValue();
            if (onValueChanged) onValueChanged(value);
            return true;
        } else if (key == VK_RETURN) {
            syncTextFromValue();
            return true;
        }
    }
    bool handled = TextInput::onKeyDown(key);
    if (handled) syncValueFromText();
    return handled;
}

bool NumberInput::onMouseDown(float x, float y) {
    if (getUpButtonRect().contains(x, y)) {
        value = std::clamp(value + step, min, max);
        syncTextFromValue();
        if (onValueChanged) onValueChanged(value);
        return true;
    }
    if (getDownButtonRect().contains(x, y)) {
        value = std::clamp(value - step, min, max);
        syncTextFromValue();
        if (onValueChanged) onValueChanged(value);
        return true;
    }
    return TextInput::onMouseDown(x, y);
}

bool NumberInput::onMouseWheel(float x, float y, float delta) {
    if (hitTest(x, y)) {
        value = std::clamp(value + (delta > 0 ? step : -step), min, max);
        syncTextFromValue();
        if (onValueChanged) onValueChanged(value);
        return true;
    }
    return false;
}

} // namespace SphereUI
