#pragma once
#include <cstdint>

namespace AureliaUI::events {

struct WindowResizeEvent {
    std::int32_t clientWidth = 0;
    std::int32_t clientHeight = 0;
};

struct WindowDpiEvent {
    float dpiX = 96.0f;
    float dpiY = 96.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
};

} // namespace AureliaUI::events
