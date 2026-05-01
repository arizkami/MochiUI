#pragma once
#include <gui/Layout.hpp>
#include <core/events/PointerEvent.hpp>
#include <core/events/WheelEvent.hpp>
#include <core/events/KeyEvent.hpp>
#include <core/events/TextEvent.hpp>

namespace AureliaUI::events {

// Bridge typed events → existing FlexNode virtuals (until nodes take structs natively).

inline bool dispatchPointerMove(FlexNode& root, PointerEvent& e) {
    const bool r = root.onMouseMove(e.x, e.y);
    e.handled = r;
    return r;
}

inline void dispatchPointerPrimaryUp(FlexNode& root, PointerEvent& e) {
    root.onMouseUp(e.x, e.y);
    e.handled = true;
}

inline bool dispatchPointerPrimaryDown(FlexNode& root, PointerEvent& e) {
    const bool r = root.onMouseDown(e.x, e.y);
    e.handled = r;
    return r;
}

inline bool dispatchPointerSecondaryDown(FlexNode& root, PointerEvent& e) {
    const bool r = root.onRightDown(e.x, e.y);
    e.handled = r;
    return r;
}

inline bool dispatchWheel(FlexNode& root, WheelEvent& e) {
    const bool r = root.onMouseWheel(e.x, e.y, e.deltaY);
    e.handled = r;
    return r;
}

inline bool dispatchKeyDown(FlexNode& root, KeyEvent& e) {
    const bool r = root.onKeyDown(e.keyCode);
    e.handled = r;
    return r;
}

inline bool dispatchTextInput(FlexNode& root, TextEvent& e) {
    const bool r = root.onChar(e.codeUnit);
    e.handled = r;
    return r;
}

} // namespace AureliaUI::events
