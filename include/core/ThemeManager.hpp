#pragma once
#include <utils/Misc/ThemeSwitcher.hpp>

namespace SphereUI {

class ThemeManager {
public:
    static ThemeManager& getInstance() {
        static ThemeManager instance;
        return instance;
    }

    void setTheme(ThemeType type) {
        ThemeSwitcher::getInstance().setTheme(type);
    }

    void toggleDarkMode() {
        auto current = ThemeSwitcher::getInstance().getCurrentTheme();
        if (current == ThemeType::Dark) setTheme(ThemeType::Light);
        else setTheme(ThemeType::Dark);
    }
};

} // namespace SphereUI
