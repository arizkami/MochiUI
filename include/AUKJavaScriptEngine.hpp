#pragma once

#include <AUKGraphicComponents.hpp>
#include <memory>
#include <string>
#include <string_view>

namespace AureliaUI {

// ── API export macro ─────────────────────────────────────────────────────────
#ifdef AUKJS_BUILD_SHARED
    #define AUKJS_API __declspec(dllexport)
#elif defined(AUKJS_SHARED)
    #define AUKJS_API __declspec(dllimport)
#else
    #define AUKJS_API
#endif

// Embeds a V8 isolate + default context. AureliaUI bindings are installed via
// `installAureliaUIGlobal()` (see implementation).
class AUKJS_API JavaScriptEngine {
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

    /** Install `globalThis.AureliaUI` native helpers for the React bridge. */
    void installAureliaUIGlobal();

    /** Take the root node set by `AureliaUI.setRoot(handle)` (single-shot). */
    FlexNode::Ptr takePendingRoot();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::string lastError_;
};

} // namespace AureliaUI
