#include <include/core/Application.hpp>
#include <include/utils/FontManager/FontMgr.hpp>
#include <include/utils/Misc/ThemeSwitcher.hpp>

namespace MochiUI {

Application& Application::getInstance() {
    static Application instance;
    return instance;
}

void Application::init() {
    FontManager::getInstance().initialize();
    ThemeSwitcher::getInstance().applyTheme();
}

} // namespace MochiUI
