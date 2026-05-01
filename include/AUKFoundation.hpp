#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include <vector>

// ── Version ───────────────────────────────────────────────────────────────────
#define AUK_VERSION_MAJOR  0
#define AUK_VERSION_MINOR  1
#define AUK_VERSION_PATCH  0
#define AUK_VERSION_STRING "0.1.0"

// ── Platform detection ────────────────────────────────────────────────────────
#ifdef _WIN32
    #define AUK_PLATFORM_WINDOWS 1
    #ifdef _WIN64
        #define AUK_PLATFORM_WIN64 1
    #endif
#endif

// ── Event payloads (pointer, key, Skia, layout phases, …) ─────────────────────
#include <core/events/Events.hpp>

// ── API export macro ──────────────────────────────────────────────────────────
#ifdef AUK_BUILD_SHARED
    #define AUK_API __declspec(dllexport)
#elif defined(AUK_SHARED)
    #define AUK_API __declspec(dllimport)
#else
    #define AUK_API
#endif

// ── Skia drawing types (SkCanvas, SkPaint, SkColor, SkPath, SkFont …) ─────────
// Re-exported so callers never need to #include Skia headers directly.
#include <gui/SkiaDraw.hpp>

// ── Color API (AUKColor::Hex, ::RGB, ::Float, ::HSL + manipulation) ───────────
#include <AUKColor.hpp>

// ── Layout primitives (FlexNode, LayoutStyle, Size …) ────────────────────────
#include <gui/Layout.hpp>

// ── Theme tokens (Theme::Background, Theme::Accent …) ────────────────────────
#include <gui/Theme.hpp>
