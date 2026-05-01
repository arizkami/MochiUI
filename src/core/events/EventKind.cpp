#include <core/events/EventKind.hpp>

namespace AureliaUI::events {

const char* eventKindName(EventKind kind) noexcept {
    switch (kind) {
    case EventKind::None: return "None";
    case EventKind::PointerMove: return "PointerMove";
    case EventKind::PointerDown: return "PointerDown";
    case EventKind::PointerUp: return "PointerUp";
    case EventKind::PointerEnter: return "PointerEnter";
    case EventKind::PointerLeave: return "PointerLeave";
    case EventKind::Wheel: return "Wheel";
    case EventKind::KeyDown: return "KeyDown";
    case EventKind::KeyUp: return "KeyUp";
    case EventKind::TextInput: return "TextInput";
    case EventKind::WindowResize: return "WindowResize";
    case EventKind::WindowFocusIn: return "WindowFocusIn";
    case EventKind::WindowFocusOut: return "WindowFocusOut";
    case EventKind::WindowDpiChanged: return "WindowDpiChanged";
    case EventKind::WindowCloseRequest: return "WindowCloseRequest";
    case EventKind::SkiaSurfacePaint: return "SkiaSurfacePaint";
    case EventKind::SkiaSurfaceResized: return "SkiaSurfaceResized";
    case EventKind::SkiaContextLost: return "SkiaContextLost";
    case EventKind::LayoutSubtreeDirty: return "LayoutSubtreeDirty";
    case EventKind::LayoutPassRequested: return "LayoutPassRequested";
    case EventKind::LayoutPassComplete: return "LayoutPassComplete";
    default: return "Unknown";
    }
}

} // namespace AureliaUI::events
