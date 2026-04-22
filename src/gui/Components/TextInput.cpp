#include <include/gui/Components/TextInput.hpp>
#include <windows.h>

namespace MochiUI {

TextInput::TextInput() {
    style.widthMode = SizingMode::Flex;
    style.heightMode = SizingMode::Fixed;
    style.height = 36.0f;
    style.padding = 10.0f;
    style.borderRadius = 6.0f;
}

Size TextInput::measure(Size available) {
    float w = (style.widthMode == SizingMode::Fixed) ? style.width : available.width;
    float h = (style.heightMode == SizingMode::Fixed) ? style.height : 36.0f;
    return { w, h };
}

void TextInput::draw(SkCanvas* canvas) {
    // 1. Draw Background
    SkPaint bgPaint;
    bgPaint.setAntiAlias(true);
    bgPaint.setColor(backgroundColor);
    canvas->drawRoundRect(frame, style.borderRadius, style.borderRadius, bgPaint);

    // 2. Draw Border (Accent color if focused)
    SkPaint borderPaint;
    borderPaint.setAntiAlias(true);
    borderPaint.setStyle(SkPaint::kStroke_Style);
    borderPaint.setStrokeWidth(isFocused ? 2.0f : 1.0f);
    borderPaint.setColor(isFocused ? focusColor : borderColor);
    
    SkRect borderRect = frame;
    if (isFocused) borderRect.inset(0.5f, 0.5f);
    canvas->drawRoundRect(borderRect, style.borderRadius, style.borderRadius, borderPaint);

    // 3. Draw Text or Placeholder
    SkFont font = FontManager::getInstance().createFont(FontManager::DEFAULT_FONT, fontSize);
    SkPaint textPaint;
    textPaint.setAntiAlias(true);
    
    float textX = frame.left() + style.padding;
    float textY = frame.centerY() + fontSize / 2.0f - 2.0f;

    if (text.empty() && !isFocused) {
        textPaint.setColor(placeholderColor);
        canvas->drawSimpleText(placeholder.c_str(), placeholder.size(), SkTextEncoding::kUTF8, 
                               textX, textY, font, textPaint);
    } else {
        textPaint.setColor(textColor);
        canvas->drawSimpleText(text.c_str(), text.size(), SkTextEncoding::kUTF8, 
                               textX, textY, font, textPaint);
        
        // 4. Draw Cursor if focused
        if (isFocused) {
            uint32_t currentTime = GetTickCount();
            if (currentTime - lastBlinkTime > 500) {
                showCursor = !showCursor;
                lastBlinkTime = currentTime;
            }
            
            if (showCursor) {
                float cursorX = textX;
                if (cursorIndex > 0) {
                    cursorX += font.measureText(text.c_str(), cursorIndex, SkTextEncoding::kUTF8);
                }
                
                SkPaint cursorPaint;
                cursorPaint.setColor(focusColor);
                cursorPaint.setStrokeWidth(1.5f);
                canvas->drawLine(cursorX, textY - fontSize + 2.0f, cursorX, textY + 2.0f, cursorPaint);
            }
        }
    }
}

bool TextInput::onMouseDown(float x, float y) {
    if (hitTest(x, y)) {
        isFocused = true;
        showCursor = true;
        lastBlinkTime = GetTickCount();
        
        // Simple cursor positioning
        SkFont font = FontManager::getInstance().createFont(FontManager::DEFAULT_FONT, fontSize);
        float relX = x - (frame.left() + style.padding);
        
        cursorIndex = 0;
        float currentX = 0;
        for (size_t i = 0; i < text.size(); ++i) {
            float charW = font.measureText(&text[i], 1, SkTextEncoding::kUTF8);
            if (relX < currentX + charW / 2.0f) break;
            currentX += charW;
            cursorIndex = i + 1;
        }
        return true;
    }
    return false;
}

bool TextInput::onChar(uint32_t charCode) {
    if (!isFocused) return false;

    if (charCode == 8) { // Backspace
        if (cursorIndex > 0) {
            text.erase(cursorIndex - 1, 1);
            cursorIndex--;
            if (onChanged) onChanged(text);
        }
    } else if (charCode == 13) { // Enter
        if (onEnter) onEnter();
    } else if (charCode >= 32) { // Normal character
        text.insert(cursorIndex, 1, (char)charCode);
        cursorIndex++;
        if (onChanged) onChanged(text);
    }
    
    showCursor = true;
    lastBlinkTime = GetTickCount();
    return true;
}

bool TextInput::onKeyDown(uint32_t key) {
    if (!isFocused) return false;

    if (key == VK_LEFT) {
        if (cursorIndex > 0) cursorIndex--;
    } else if (key == VK_RIGHT) {
        if (cursorIndex < text.size()) cursorIndex++;
    } else if (key == VK_DELETE) {
        if (cursorIndex < text.size()) {
            text.erase(cursorIndex, 1);
            if (onChanged) onChanged(text);
        }
    } else if (key == VK_HOME) {
        cursorIndex = 0;
    } else if (key == VK_END) {
        cursorIndex = text.size();
    } else {
        return false;
    }

    showCursor = true;
    lastBlinkTime = GetTickCount();
    return true;
}

bool TextInput::needsRedraw() {
    return isFocused; // Keep blinking
}

} // namespace MochiUI
