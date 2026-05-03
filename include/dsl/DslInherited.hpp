#pragma once
#include <AUKColor.hpp>
#include <gui/Theme.hpp>
#include <string>

namespace AureliaUI {

// Font / colour cascade state shared between the YAML style applier and the
// libcss declaration applier.
struct DslInherited {
    std::string fontFamily;
    AUKColor    color    = Theme::TextPrimary;
    float       fontSize = 14.0f;
    bool        bold     = false;
    bool        hasColor = false;
};

} // namespace AureliaUI
