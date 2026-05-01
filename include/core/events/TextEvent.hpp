#pragma once
#include <cstdint>

namespace AureliaUI::events {

// Single UTF-32 code unit (WM_CHAR–style). Surrogate pairs should be merged by the host if required.
struct TextEvent {
    std::uint32_t codeUnit = 0;
    bool handled = false;
};

} // namespace AureliaUI::events
