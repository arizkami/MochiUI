#pragma once
#include <SPHXApplication.hpp>

// ── Abstract window interface (IWindow, WindowMode) ───────────────────────────
#include <core/Window.hpp>
#include <gui/GridLayout.hpp>
#include <core/events/FlexRootDispatch.hpp>

// ── Platform window backend ───────────────────────────────────────────────────
#ifdef SPHX_PLATFORM_WINDOWS
#include <platform/windows/Window.hpp>
#endif

namespace SphereUI {

// Convenience aliases — mirrors Qt's QApplication / QWidget pattern
using App = Application;

#ifdef SPHX_PLATFORM_WINDOWS
using Window = Win32Window;
#endif

} // namespace SphereUI
