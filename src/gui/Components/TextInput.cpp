#include <include/gui/Components/TextInput.hpp>
#include <windows.h>

namespace MochiUI {

namespace {
    void SetClipboardText(const std::string& text) {
        if (OpenClipboard(nullptr)) {
            EmptyClipboard();
            int wsize = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, wsize * sizeof(wchar_t));
            if (hMem) {
                wchar_t* wstr = (wchar_t*)GlobalLock(hMem);
                MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wstr, wsize);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
            }
            CloseClipboard();
        }
    }

    std::string GetClipboardText() {
        std::string result;
        if (OpenClipboard(nullptr)) {
            HANDLE hData = GetClipboardData(CF_UNICODETEXT);
            if (hData) {
                wchar_t* wstr = (wchar_t*)GlobalLock(hData);
                if (wstr) {
                    int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
                    if (size > 0) {
                        result.resize(size - 1);
                        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], size, nullptr, nullptr);
                    }
                    GlobalUnlock(hData);
                }
            }
            CloseClipboard();
        }
        return result;
    }
}


Size TextInput::measure(Size available) {
    float h = fontSize + style.padding * 2.0f + 10.0f;
    float w = 150.0f; // Default width
    return { w, h };
}

bool TextInput::hasSelection() const {
    return selectionAnchor != std::string::npos && selectionAnchor != cursorIndex;
}

size_t TextInput::getSelectionStart() const {
    return std::min(cursorIndex, selectionAnchor);
}

size_t TextInput::getSelectionEnd() const {
    return std::max(cursorIndex, selectionAnchor);
}

void TextInput::deleteSelection() {
    if (!hasSelection()) return;
    size_t start = getSelectionStart();
    size_t end = getSelectionEnd();
    text.erase(start, end - start);
    cursorIndex = start;
    selectionAnchor = std::string::npos;
    if (onChanged) onChanged(text);
}

size_t TextInput::getCursorIndexFromPosition(float x) {
    float relX = x - (frame.left() + style.padding);
    size_t bestIndex = 0;
    float lastWidth = 0.0f;
    for (size_t i = 0; i <= text.size(); ++i) {
        if (i < text.size() && (text[i] & 0xC0) == 0x80) continue;
        
        float widthUpToI = FontManager::getInstance().measureText(text, i, fontSize);
        if (relX < widthUpToI) {
            if (relX < (lastWidth + (widthUpToI - lastWidth) / 2.0f)) {
                // keep bestIndex as previous valid boundary
            } else {
                bestIndex = i;
            }
            return bestIndex;
        }
        bestIndex = i;
        lastWidth = widthUpToI;
    }
    return bestIndex;
}

void TextInput::draw(SkCanvas* canvas) {
    drawSelf(canvas);

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
    SkPaint textPaint;
    textPaint.setAntiAlias(true);
    
    float textX = frame.left() + style.padding + 4.0f;
    float textY = frame.centerY() + fontSize / 2.0f - 2.0f;

    if (text.empty() && !isFocused) {
        textPaint.setColor(placeholderColor);
        FontManager::getInstance().drawText(canvas, placeholder, textX, textY, fontSize, textPaint);
    } else {
        // Draw Selection Background
        if (hasSelection() && isFocused) {
            float startX = textX + FontManager::getInstance().measureText(text, getSelectionStart(), fontSize);
            float endX = textX + FontManager::getInstance().measureText(text, getSelectionEnd(), fontSize);
            
            SkPaint selPaint;
            selPaint.setColor(SkColorSetA(focusColor, 100)); // Semi-transparent accent
            canvas->drawRect(SkRect::MakeLTRB(startX, textY - fontSize + 2.0f, endX, textY + 4.0f), selPaint);
        }

        textPaint.setColor(textColor);
        FontManager::getInstance().drawText(canvas, text, textX, textY, fontSize, textPaint);
        
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
                    cursorX += FontManager::getInstance().measureText(text, cursorIndex, fontSize);
                }
                
                SkPaint cursorPaint;
                cursorPaint.setColor(focusColor);
                cursorPaint.setStrokeWidth(1.5f);
                canvas->drawLine(cursorX, textY - fontSize + 2.0f, cursorX, textY + 2.0f, cursorPaint);
            }
        }
    }

    drawChildren(canvas);
}

bool TextInput::onMouseDown(float x, float y) {
    if (hitTest(x, y)) {
        isFocused = true;
        showCursor = true;
        lastBlinkTime = GetTickCount();
        
        bool shiftPressed = GetKeyState(VK_SHIFT) & 0x8000;
        size_t newIndex = getCursorIndexFromPosition(x);

        if (shiftPressed) {
            if (selectionAnchor == std::string::npos) {
                selectionAnchor = cursorIndex;
            }
        } else {
            selectionAnchor = newIndex; // start dragging anchor
        }
        
        cursorIndex = newIndex;
        isDragging = true;
        return true;
    }
    
    // Clear focus and selection if clicked outside
    isFocused = false;
    selectionAnchor = std::string::npos;
    return false;
}

bool TextInput::onMouseMove(float x, float y) {
    bool handled = FlexNode::onMouseMove(x, y);
    if (isDragging && isFocused) {
        cursorIndex = getCursorIndexFromPosition(x);
        showCursor = true;
        lastBlinkTime = GetTickCount();
        handled = true;
    }
    return handled;
}

void TextInput::onMouseUp(float x, float y) {
    isDragging = false;
    if (selectionAnchor == cursorIndex) {
        selectionAnchor = std::string::npos;
    }
    FlexNode::onMouseUp(x, y);
}

bool TextInput::onChar(uint32_t charCode) {
    if (!isFocused) return false;
    
    // Ignore control characters meant for shortcuts
    if (charCode < 32 && charCode != 8 && charCode != 13) return false;

    if (charCode == 8) { // Backspace
        if (hasSelection()) {
            deleteSelection();
        } else if (cursorIndex > 0) {
            size_t delEnd = cursorIndex;
            cursorIndex--;
            while (cursorIndex > 0 && (text[cursorIndex] & 0xC0) == 0x80) {
                cursorIndex--;
            }
            text.erase(cursorIndex, delEnd - cursorIndex);
            if (onChanged) onChanged(text);
        }
        showCursor = true;
        lastBlinkTime = GetTickCount();
        return true;
    } else if (charCode == 13) { // Enter
        if (onEnter) onEnter();
    } else if (charCode >= 32) { // Normal character
        uint32_t codepoint = charCode;
        
        // Handle UTF-16 surrogates from Windows WM_CHAR
        if (charCode >= 0xD800 && charCode <= 0xDBFF) {
            highSurrogate = (uint16_t)charCode;
            return true; // Wait for low surrogate
        }
        if (charCode >= 0xDC00 && charCode <= 0xDFFF) {
            if (highSurrogate != 0) {
                codepoint = 0x10000 + (((highSurrogate - 0xD800) << 10) | (charCode - 0xDC00));
                highSurrogate = 0;
            } else {
                return true; // Invalid surrogate pair
            }
        } else {
            highSurrogate = 0;
        }

        // Convert codepoint to UTF-8
        std::string utf8Str;
        if (codepoint <= 0x7F) {
            utf8Str += (char)codepoint;
        } else if (codepoint <= 0x7FF) {
            utf8Str += (char)(0xC0 | ((codepoint >> 6) & 0x1F));
            utf8Str += (char)(0x80 | (codepoint & 0x3F));
        } else if (codepoint <= 0xFFFF) {
            utf8Str += (char)(0xE0 | ((codepoint >> 12) & 0x0F));
            utf8Str += (char)(0x80 | ((codepoint >> 6) & 0x3F));
            utf8Str += (char)(0x80 | (codepoint & 0x3F));
        } else if (codepoint <= 0x10FFFF) {
            utf8Str += (char)(0xF0 | ((codepoint >> 18) & 0x07));
            utf8Str += (char)(0x80 | ((codepoint >> 12) & 0x3F));
            utf8Str += (char)(0x80 | ((codepoint >> 6) & 0x3F));
            utf8Str += (char)(0x80 | (codepoint & 0x3F));
        }

        if (hasSelection()) deleteSelection();

        text.insert(cursorIndex, utf8Str);
        cursorIndex += utf8Str.length();
        if (onChanged) onChanged(text);
    }
    
    showCursor = true;
    lastBlinkTime = GetTickCount();
    return true;
}

bool TextInput::onKeyDown(uint32_t key) {
    if (!isFocused) return false;

    bool ctrlPressed = GetKeyState(VK_CONTROL) & 0x8000;
    bool shiftPressed = GetKeyState(VK_SHIFT) & 0x8000;

    if (ctrlPressed) {
        if (key == 'A') { // Ctrl+A
            selectionAnchor = 0;
            cursorIndex = text.size();
            showCursor = true;
            lastBlinkTime = GetTickCount();
            return true;
        } else if (key == 'C') { // Ctrl+C
            if (hasSelection()) {
                std::string selected = text.substr(getSelectionStart(), getSelectionEnd() - getSelectionStart());
                SetClipboardText(selected);
            }
            return true;
        } else if (key == 'X') { // Ctrl+X
            if (hasSelection()) {
                std::string selected = text.substr(getSelectionStart(), getSelectionEnd() - getSelectionStart());
                SetClipboardText(selected);
                deleteSelection();
                showCursor = true;
                lastBlinkTime = GetTickCount();
            }
            return true;
        } else if (key == 'V') { // Ctrl+V
            std::string clip = GetClipboardText();
            if (!clip.empty()) {
                if (hasSelection()) deleteSelection();
                text.insert(cursorIndex, clip);
                cursorIndex += clip.size();
                if (onChanged) onChanged(text);
                showCursor = true;
                lastBlinkTime = GetTickCount();
            }
            return true;
        }
    }

    if (key == VK_LEFT || key == VK_RIGHT || key == VK_HOME || key == VK_END) {
        if (shiftPressed && selectionAnchor == std::string::npos) {
            selectionAnchor = cursorIndex;
        } else if (!shiftPressed) {
            selectionAnchor = std::string::npos;
        }

        if (key == VK_LEFT) {
            if (cursorIndex > 0) {
                cursorIndex--;
                while (cursorIndex > 0 && (text[cursorIndex] & 0xC0) == 0x80) {
                    cursorIndex--;
                }
            }
        } else if (key == VK_RIGHT) {
            if (cursorIndex < text.size()) {
                cursorIndex++;
                while (cursorIndex < text.size() && (text[cursorIndex] & 0xC0) == 0x80) {
                    cursorIndex++;
                }
            }
        } else if (key == VK_HOME) {
            cursorIndex = 0;
        } else if (key == VK_END) {
            cursorIndex = text.size();
        }
    } else if (key == VK_DELETE) {
        if (hasSelection()) {
            deleteSelection();
        } else if (cursorIndex < text.size()) {
            size_t delEnd = cursorIndex + 1;
            while (delEnd < text.size() && (text[delEnd] & 0xC0) == 0x80) {
                delEnd++;
            }
            text.erase(cursorIndex, delEnd - cursorIndex);
            if (onChanged) onChanged(text);
        }
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
