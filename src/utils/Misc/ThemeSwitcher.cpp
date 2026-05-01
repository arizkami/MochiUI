#include <utils/Misc/ThemeSwitcher.hpp>
#include <gui/Theme.hpp>
#include <windows.h>

namespace AureliaUI {

ThemeSwitcher& ThemeSwitcher::getInstance() {
    static ThemeSwitcher instance;
    return instance;
}

bool ThemeSwitcher::isWindowsInDarkMode() {
    HKEY  hKey;
    DWORD value = 0;
    DWORD size  = sizeof(DWORD);

    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &hKey);

    if (result == ERROR_SUCCESS) {
        result = RegQueryValueExW(hKey, L"AppsUseLightTheme",
                                  nullptr, nullptr, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
        if (result == ERROR_SUCCESS)
            return (value == 0); // 0 = dark, 1 = light
    }
    return true; // default to dark
}

// Helper: copy a theme namespace into the active Theme:: globals
#define APPLY_THEME(NS)                              \
    Theme::Background    = NS::Background;           \
    Theme::Sidebar       = NS::Sidebar;              \
    Theme::MenuBar       = NS::MenuBar;              \
    Theme::Accent        = NS::Accent;               \
    Theme::TextPrimary   = NS::TextPrimary;          \
    Theme::TextSecondary = NS::TextSecondary;        \
    Theme::HoverOverlay  = NS::HoverOverlay;         \
    Theme::Card          = NS::Card;                 \
    Theme::Border        = NS::Border;               \
    Theme::Shadow        = NS::Shadow

void ThemeSwitcher::applyTheme() {
    ThemeType effective = currentTheme;

    if (effective == ThemeType::Auto)
        effective = isWindowsInDarkMode() ? ThemeType::Dark : ThemeType::Light;

    switch (effective) {
        case ThemeType::Light:      APPLY_THEME(LightTheme);     Theme::BorderRadius = 4.0f; break;
        case ThemeType::System:     APPLY_THEME(SystemTheme);    Theme::BorderRadius = 0.0f; break;
        case ThemeType::Minimal:    APPLY_THEME(MinimalTheme);   Theme::BorderRadius = 0.0f; break;
        case ThemeType::Md3Dark:    APPLY_THEME(Md3darkTheme);   Theme::BorderRadius = 8.0f; break;
        case ThemeType::Md3Light:   APPLY_THEME(Md3lightTheme);  Theme::BorderRadius = 8.0f; break;
        case ThemeType::WinuiDark:  APPLY_THEME(WinuidarkTheme); Theme::BorderRadius = 4.0f; break;
        case ThemeType::WinuiLight: APPLY_THEME(WinuilightTheme);Theme::BorderRadius = 4.0f; break;
        default:                    APPLY_THEME(DarkTheme);      Theme::BorderRadius = 4.0f; break;
    }

    Theme::BorderWidth = 1.0f;
}

void ThemeSwitcher::setTheme(ThemeType theme) {
    if (currentTheme != theme) {
        currentTheme = theme;
        applyTheme();
        notifyThemeChanged();
    }
}

void ThemeSwitcher::setTheme(const std::string& themeName) {
    ThemeType t = ThemeType::Auto;
    if      (themeName == "dark")        t = ThemeType::Dark;
    else if (themeName == "light")       t = ThemeType::Light;
    else if (themeName == "system")      t = ThemeType::System;
    else if (themeName == "minimal")     t = ThemeType::Minimal;
    else if (themeName == "md3dark")     t = ThemeType::Md3Dark;
    else if (themeName == "md3light")    t = ThemeType::Md3Light;
    else if (themeName == "winuidark")   t = ThemeType::WinuiDark;
    else if (themeName == "winuilight")  t = ThemeType::WinuiLight;
    setTheme(t);
}

std::string ThemeSwitcher::getCurrentThemeName() const {
    switch (currentTheme) {
        case ThemeType::Dark:       return "dark";
        case ThemeType::Light:      return "light";
        case ThemeType::System:     return "system";
        case ThemeType::Minimal:    return "minimal";
        case ThemeType::Md3Dark:    return "md3dark";
        case ThemeType::Md3Light:   return "md3light";
        case ThemeType::WinuiDark:  return "winuidark";
        case ThemeType::WinuiLight: return "winuilight";
        default:                    return "auto";
    }
}

void ThemeSwitcher::detectAndApplyWindowsTheme() {
    if (currentTheme == ThemeType::Auto) {
        applyTheme();
        notifyThemeChanged();
    }
}

void ThemeSwitcher::registerThemeChangeCallback(std::function<void(ThemeType)> callback) {
    callbacks.push_back(callback);
}

void ThemeSwitcher::notifyThemeChanged() {
    for (auto& cb : callbacks) cb(currentTheme);
}

} // namespace AureliaUI
