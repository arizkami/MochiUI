#pragma once

namespace AureliaUI::events {

// Wheel deltas: host-defined units (e.g. Windows wheel delta / WHEEL_DELTA).
struct WheelEvent {
    float x = 0;
    float y = 0;
    float deltaX = 0;
    float deltaY = 0;
    bool precise = false; // true for high-res trackpad deltas when known
    bool handled = false;
};

} // namespace AureliaUI::events
