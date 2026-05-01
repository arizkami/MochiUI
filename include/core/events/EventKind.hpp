#pragma once
#include <cstdint>

namespace AureliaUI::events {

// Stable categories for routing, logging, and future event buses.
enum class EventKind : std::uint32_t {
    None = 0,

    // Pointer / hit-testing (client coordinates, DIPs)
    PointerMove,
    PointerDown,
    PointerUp,
    PointerEnter,
    PointerLeave,

    // Scroll wheel / trackpad
    Wheel,

    // Keyboard
    KeyDown,
    KeyUp,
    TextInput,

    // Top-level window / host
    WindowResize,
    WindowFocusIn,
    WindowFocusOut,
    WindowDpiChanged,
    WindowCloseRequest,

    // Skia surface / frame
    SkiaSurfacePaint,
    SkiaSurfaceResized,
    SkiaContextLost,

    // Layout pipeline (Yoga-driven trees; no Yoga headers required here)
    LayoutSubtreeDirty,
    LayoutPassRequested,
    LayoutPassComplete,
};

const char* eventKindName(EventKind kind) noexcept;

} // namespace AureliaUI::events
