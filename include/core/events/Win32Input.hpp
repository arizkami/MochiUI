#pragma once

#if defined(_WIN32)

#include <cstdint>
#include <windows.h>
#include <core/events/PointerEvent.hpp>
#include <core/events/WheelEvent.hpp>
#include <core/events/KeyEvent.hpp>
#include <core/events/TextEvent.hpp>

namespace AureliaUI::events {

inline float clientXFromLParam(LPARAM lp) noexcept {
    return static_cast<float>(static_cast<short>(LOWORD(lp)));
}

inline float clientYFromLParam(LPARAM lp) noexcept {
    return static_cast<float>(static_cast<short>(HIWORD(lp)));
}

inline PointerButton pointerButtonsFromKeyState() noexcept {
    PointerButton b = PointerButton::None;
    if ((GetKeyState(VK_LBUTTON) & 0x8000) != 0) {
        b = b | PointerButton::Primary;
    }
    if ((GetKeyState(VK_RBUTTON) & 0x8000) != 0) {
        b = b | PointerButton::Secondary;
    }
    if ((GetKeyState(VK_MBUTTON) & 0x8000) != 0) {
        b = b | PointerButton::Middle;
    }
    if ((GetKeyState(VK_XBUTTON1) & 0x8000) != 0) {
        b = b | PointerButton::X1;
    }
    if ((GetKeyState(VK_XBUTTON2) & 0x8000) != 0) {
        b = b | PointerButton::X2;
    }
    return b;
}

inline PointerEvent pointerEventFromClient(LPARAM lp, PointerButton buttons,
                                           PointerButton changed = PointerButton::None,
                                           bool pressed = false) noexcept {
    PointerEvent e;
    e.x = clientXFromLParam(lp);
    e.y = clientYFromLParam(lp);
    e.buttons = buttons;
    e.changedButton = changed;
    e.pressed = pressed;
    return e;
}

inline WheelEvent wheelEventFromMouseWheel(HWND hwnd, WPARAM wp, LPARAM lp) noexcept {
    WheelEvent e;
    POINT pt = {static_cast<LONG>(static_cast<short>(LOWORD(lp))),
                static_cast<LONG>(static_cast<short>(HIWORD(lp)))};
    ScreenToClient(hwnd, &pt);
    e.x = static_cast<float>(pt.x);
    e.y = static_cast<float>(pt.y);
    e.deltaX = 0.0f;
    e.deltaY = static_cast<float>(static_cast<std::int16_t>(HIWORD(wp)))
        / static_cast<float>(WHEEL_DELTA);
    e.precise = false;
    return e;
}

inline KeyEvent keyEventFromKeyDown(WPARAM wp, LPARAM lp) noexcept {
    KeyEvent e;
    e.keyCode = static_cast<std::uint32_t>(wp);
    e.scanCode = (static_cast<std::uint32_t>(lp) >> 16) & 0xFFu;
    e.repeat = (lp & (1u << 30)) != 0;
    e.extendedKey = (lp & (1u << 24)) != 0;
    e.altDown = (GetKeyState(VK_MENU) & 0x8000) != 0;
    e.ctrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    e.shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    return e;
}

inline TextEvent textEventFromWmChar(WPARAM wp) noexcept {
    TextEvent e;
    e.codeUnit = static_cast<std::uint32_t>(wp);
    return e;
}

} // namespace AureliaUI::events

#endif // _WIN32
