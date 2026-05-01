#pragma once
#include <cstdint>
#include <include/core/SkRect.h>

class SkCanvas;

namespace AureliaUI::events {

// Issued around frame recording; canvas is valid only for the duration of the callback.
struct SkiaPaintEvent {
    SkCanvas* canvas = nullptr;
    SkRect dirtyBounds = SkRect::MakeEmpty();
};

struct SkiaSurfaceEvent {
    std::int32_t widthPx = 0;
    std::int32_t heightPx = 0;
    float dpiX = 96.0f;
    float dpiY = 96.0f;
};

} // namespace AureliaUI::events
