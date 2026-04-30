#pragma once
#include <cstdint>

namespace MochiUI::events {

// Describes phases for Yoga-backed UI trees without including yoga/Yoga.h.
enum class LayoutPhase : std::uint8_t {
    Unknown = 0,
    Measure,
    Arrange,
};

struct LayoutBoundsEvent {
    float width = 0;
    float height = 0;
    LayoutPhase phase = LayoutPhase::Unknown;
};

struct LayoutDirtyEvent {
    // Optional stable id for instrumentation (0 = unspecified).
    std::uint64_t sourceToken = 0;
};

} // namespace MochiUI::events
