#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include <vector>

// ── Version ───────────────────────────────────────────────────────────────────
#define MCK_VERSION_MAJOR  0
#define MCK_VERSION_MINOR  1
#define MCK_VERSION_PATCH  0
#define MCK_VERSION_STRING "0.1.0"

// ── Platform detection ────────────────────────────────────────────────────────
#ifdef _WIN32
    #define MCK_PLATFORM_WINDOWS 1
    #ifdef _WIN64
        #define MCK_PLATFORM_WIN64 1
    #endif
#endif

// ── Event payloads (pointer, key, Skia, layout phases, …) ─────────────────────
#include <core/events/Events.hpp>

// ── API export macro ──────────────────────────────────────────────────────────
#ifdef MCK_BUILD_SHARED
    #define MCK_API __declspec(dllexport)
#elif defined(MCK_SHARED)
    #define MCK_API __declspec(dllimport)
#else
    #define MCK_API
#endif

// ── Layout primitives (FlexNode, LayoutStyle, Size …) ────────────────────────
#include <gui/Layout.hpp>

// ── Theme tokens (Theme::Background, Theme::Accent …) ────────────────────────
#include <gui/Theme.hpp>
