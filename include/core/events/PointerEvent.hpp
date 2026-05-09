#pragma once
#include <cstdint>

namespace SphereUI::events {

enum class PointerButton : std::uint32_t {
    None = 0,
    Primary = 1u << 0,
    Secondary = 1u << 1,
    Middle = 1u << 2,
    X1 = 1u << 3,
    X2 = 1u << 4,
};

constexpr PointerButton operator|(PointerButton a, PointerButton b) noexcept {
    return static_cast<PointerButton>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

constexpr PointerButton operator&(PointerButton a, PointerButton b) noexcept {
    return static_cast<PointerButton>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}

constexpr bool pointerHas(PointerButton set, PointerButton mask) noexcept {
    return (static_cast<std::uint32_t>(set) & static_cast<std::uint32_t>(mask)) != 0;
}

// Client-area coordinates (same space as FlexNode hit-testing).
struct PointerEvent {
    float x = 0;
    float y = 0;
    PointerButton buttons = PointerButton::None;
    // For Down/Up: which logical button changed; for Move: often None.
    PointerButton changedButton = PointerButton::None;
    bool pressed = false;
    std::uint32_t clickCount = 1;
    bool handled = false;
};

} // namespace SphereUI::events
