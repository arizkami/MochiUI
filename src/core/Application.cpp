#include <include/core/Application.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <include/utils/Misc/ThemeSwitcher.hpp>
#include <windows.h>
#include <objbase.h>

namespace MochiUI {

Application& Application::getInstance() {
    static Application instance;
    return instance;
}

void Application::init() {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    FontManager::getInstance().initialize();
    ThemeSwitcher::getInstance().applyTheme();
}

} // namespace MochiUI
