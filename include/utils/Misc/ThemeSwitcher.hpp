#pragma once
#include <string>
#include <functional>
#include <vector>

namespace MochiUI {

enum class ThemeType {
    Auto,   // Automatically detect from Windows
    Dark,
    Light,
    System,
    Minimal
};

class ThemeSwitcher {
public:
    static ThemeSwitcher& getInstance();
    
    void setTheme(ThemeType theme);
    void setTheme(const std::string& themeName);
    ThemeType getCurrentTheme() const { return currentTheme; }
    std::string getCurrentThemeName() const;
    
    void applyTheme();
    void detectAndApplyWindowsTheme();
    bool isWindowsInDarkMode();
    
    void registerThemeChangeCallback(std::function<void(ThemeType)> callback);
    void notifyThemeChanged();

private:
    ThemeSwitcher() = default;
    ~ThemeSwitcher() = default;
    ThemeSwitcher(const ThemeSwitcher&) = delete;
    ThemeSwitcher& operator=(const ThemeSwitcher&) = delete;
    
    ThemeType currentTheme = ThemeType::Auto;
    std::vector<std::function<void(ThemeType)>> callbacks;
};

} // namespace MochiUI
