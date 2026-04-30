#pragma once
#include <MCKApplication.hpp>

// ── Abstract window interface (IWindow, WindowMode) ───────────────────────────
#include <core/Window.hpp>

// ── Platform window backend ───────────────────────────────────────────────────
#ifdef MCK_PLATFORM_WINDOWS
#include <platform/windows/Window.hpp>
#endif

namespace MochiUI {

// Convenience aliases — mirrors Qt's QApplication / QWidget pattern
using App = Application;

#ifdef MCK_PLATFORM_WINDOWS
using Window = Win32Window;
#endif

} // namespace MochiUI
