#pragma once
#include <cstdint>

namespace AureliaUI::events {

// Virtual-key style code (e.g. Windows VK_*); host maps platform scancode → this when needed.
struct KeyEvent {
    std::uint32_t keyCode = 0;
    std::uint32_t scanCode = 0;
    bool repeat = false;
    bool extendedKey = false;
    bool altDown = false;
    bool ctrlDown = false;
    bool shiftDown = false;
    bool handled = false;
};

} // namespace AureliaUI::events
