#pragma once

#include <AUKJavaScriptEngine.hpp>

namespace AureliaUI {

#ifdef AUR_BUILD_SHARED
    #define AUR_API __declspec(dllexport)
#elif defined(AUR_SHARED)
    #define AUR_API __declspec(dllimport)
#else
    #define AUR_API
#endif

// Execute a JS entry script (typically a bundled output from modules/reactui)
// and return the root node set via `globalThis.AureliaUI.setRoot(handle)`.
AUR_API FlexNode::Ptr RenderReactApp(JavaScriptEngine& engine,
                                    const std::string& entryScript,
                                    const std::string& filename = "reactui.bundle.js");

} // namespace AureliaUI

