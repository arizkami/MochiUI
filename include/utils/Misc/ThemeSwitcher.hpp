#pragma once
#include <string>
#include <functional>
#include <vector>

namespace SphereUI {

enum class ThemeType {
    Dark,
    Light,
};

class ThemeSwitcher {
public:
    static ThemeSwitcher& getInstance();

    void setTheme(ThemeType theme);
    void setTheme(const std::string& themeName);
    ThemeType getCurrentTheme() const { return currentTheme; }
    std::string getCurrentThemeName() const;

    void applyTheme();

    void registerThemeChangeCallback(std::function<void(ThemeType)> callback);
    void notifyThemeChanged();

private:
    ThemeSwitcher() = default;
    ~ThemeSwitcher() = default;
    ThemeSwitcher(const ThemeSwitcher&) = delete;
    ThemeSwitcher& operator=(const ThemeSwitcher&) = delete;

    ThemeType currentTheme = ThemeType::Dark;
    std::vector<std::function<void(ThemeType)>> callbacks;
};

} // namespace SphereUI
