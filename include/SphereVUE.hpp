#pragma once

#include <SPHXJavaScriptEngine.hpp>

namespace SphereUI {

#ifdef AUR_BUILD_SHARED
    #define AUR_API __declspec(dllexport)
#elif defined(AUR_SHARED)
    #define AUR_API __declspec(dllimport)
#else
    #define AUR_API
#endif

// Execute a JS entry script (typically a bundled output from modules/vueui)
// and return the root node set via `globalThis.SphereUI.setRoot(handle)`.
AUR_API FlexNode::Ptr RenderVueApp(JavaScriptEngine& engine,
                                   const std::string& entryScript,
                                   const std::string& filename = "vueui.bundle.js");

} // namespace SphereUI
