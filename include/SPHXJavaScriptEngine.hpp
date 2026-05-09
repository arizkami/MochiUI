#pragma once

#include <SPHXGraphicComponents.hpp>
#include <memory>
#include <string>
#include <string_view>

namespace SphereUI {

// ── API export macro ─────────────────────────────────────────────────────────
#ifdef SPHXJS_BUILD_SHARED
    #define SPHXJS_API __declspec(dllexport)
#elif defined(SPHXJS_SHARED)
    #define SPHXJS_API __declspec(dllimport)
#else
    #define SPHXJS_API
#endif

// Embeds a V8 isolate + default context. SphereUI bindings are installed via
// `installSphereUIGlobal()` (see implementation).
class SPHXJS_API JavaScriptEngine {
public:
    JavaScriptEngine();
    ~JavaScriptEngine();

    JavaScriptEngine(const JavaScriptEngine&)            = delete;
    JavaScriptEngine& operator=(const JavaScriptEngine&) = delete;

    bool init();
    void shutdown();

    /** Execute script in the default context. On failure returns false; see `lastError()`. */
    bool eval(std::string_view source, std::string_view filename = "<eval>");

    const std::string& lastError() const { return lastError_; }

    /** Install `globalThis.SphereUI` native helpers for JS UI bridges. */
    void installSphereUIGlobal();

    /** Compatibility entrypoint for Vue; currently installs the same host bridge. */
    void installSphereVueGlobal();

    /** Take the root node set by `SphereUI.setRoot(handle)` (single-shot). */
    FlexNode::Ptr takePendingRoot();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::string lastError_;
};

} // namespace SphereUI
