#pragma once
#include <include/core/Application.hpp>
#include <include/core/Window.hpp>

#ifdef _WIN32
#include <include/platform/windows/Window.hpp>
#endif

#include <BinaryResources.hpp>

namespace MochiUI {

using App = Application;

#ifdef _WIN32
using Window = Win32Window;
#endif

} // namespace MochiUI
