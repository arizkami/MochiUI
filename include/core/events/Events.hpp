#pragma once

// Umbrella header for SphereUI event payloads (input, window, Skia, layout/Yoga pipeline).

#include <core/events/EventKind.hpp>
#include <core/events/PointerEvent.hpp>
#include <core/events/KeyEvent.hpp>
#include <core/events/TextEvent.hpp>
#include <core/events/WheelEvent.hpp>
#include <core/events/WindowEvent.hpp>
#include <core/events/SkiaEvent.hpp>
#include <core/events/LayoutEvent.hpp>

// For FlexNode routing helpers, include <core/events/FlexRootDispatch.hpp> separately
// (it pulls gui/Layout.hpp and must not be included from here to avoid include cycles).
