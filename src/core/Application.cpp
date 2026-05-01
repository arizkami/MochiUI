#include <core/Application.hpp>
#include <utils/FontManager/FontMgr.hpp>
#include <utils/Misc/ThemeSwitcher.hpp>
#include <windows.h>
#include <objbase.h>

namespace AureliaUI {

Application& Application::getInstance() {
    static Application instance;
    return instance;
}

void Application::init() {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    FontManager::getInstance().initialize();
    ThemeSwitcher::getInstance().applyTheme();
}

} // namespace AureliaUI
