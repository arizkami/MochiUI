#include <include/utils/Misc/ThemeSwitcher.hpp>
#include <include/gui/Theme.hpp>
#include <windows.h>

namespace MochiUI {

ThemeSwitcher& ThemeSwitcher::getInstance() {
    static ThemeSwitcher instance;
    return instance;
}

bool ThemeSwitcher::isWindowsInDarkMode() {
    // Query Windows registry for dark mode setting
    HKEY hKey;
    DWORD value = 0;
    DWORD size = sizeof(DWORD);
    
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0,
        KEY_READ,
        &hKey
    );
    
    if (result == ERROR_SUCCESS) {
        result = RegQueryValueExW(
            hKey,
            L"AppsUseLightTheme",
            nullptr,
            nullptr,
            (LPBYTE)&value,
            &size
        );
        RegCloseKey(hKey);
        
        if (result == ERROR_SUCCESS) {
            // 0 = Dark mode, 1 = Light mode
            return (value == 0);
        }
    }
    
    // Default to dark mode if detection fails
    return true;
}

void ThemeSwitcher::applyTheme() {
    ThemeType effectiveTheme = currentTheme;
    
    // If Auto, detect from Windows
    if (effectiveTheme == ThemeType::Auto) {
        effectiveTheme = isWindowsInDarkMode() ? ThemeType::Dark : ThemeType::Light;
    }
    
    // Apply the theme by copying colors from the appropriate namespace
    if (effectiveTheme == ThemeType::Light) {
        Theme::Background    = LightTheme::Background;
        Theme::Sidebar       = LightTheme::Sidebar;
        Theme::MenuBar       = LightTheme::MenuBar;
        Theme::Accent        = LightTheme::Accent;
        Theme::TextPrimary   = LightTheme::TextPrimary;
        Theme::TextSecondary = LightTheme::TextSecondary;
        Theme::HoverOverlay  = LightTheme::HoverOverlay;
        Theme::Card          = LightTheme::Card;
        Theme::Border        = LightTheme::Border;
        Theme::Shadow        = LightTheme::Shadow;
        
        Theme::BorderRadius = 0.0f;
        Theme::BorderWidth = 1.0f;
    } else if (effectiveTheme == ThemeType::System) {
        Theme::Background    = SystemTheme::Background;
        Theme::Sidebar       = SystemTheme::Sidebar;
        Theme::MenuBar       = SystemTheme::MenuBar;
        Theme::Accent        = SystemTheme::Accent;
        Theme::TextPrimary   = SystemTheme::TextPrimary;
        Theme::TextSecondary = SystemTheme::TextSecondary;
        Theme::HoverOverlay  = SystemTheme::HoverOverlay;
        Theme::Card          = SystemTheme::Card;
        Theme::Border        = SystemTheme::Border;
        Theme::Shadow        = SystemTheme::Shadow;
        
        Theme::BorderRadius = 0.0f; // Sharp
        Theme::BorderWidth = 1.0f;
    } else if (effectiveTheme == ThemeType::Minimal) {
        Theme::Background    = MinimalTheme::Background;
        Theme::Sidebar       = MinimalTheme::Sidebar;
        Theme::MenuBar       = MinimalTheme::MenuBar;
        Theme::Accent        = MinimalTheme::Accent;
        Theme::TextPrimary   = MinimalTheme::TextPrimary;
        Theme::TextSecondary = MinimalTheme::TextSecondary;
        Theme::HoverOverlay  = MinimalTheme::HoverOverlay;
        Theme::Card          = MinimalTheme::Card;
        Theme::Border        = MinimalTheme::Border;
        Theme::Shadow        = MinimalTheme::Shadow;
        
        Theme::BorderRadius = 0.0f; // Flat for minimal look
        Theme::BorderWidth = 1.0f;
    } else {
        Theme::Background    = DarkTheme::Background;
        Theme::Sidebar       = DarkTheme::Sidebar;
        Theme::MenuBar       = DarkTheme::MenuBar;
        Theme::Accent        = DarkTheme::Accent;
        Theme::TextPrimary   = DarkTheme::TextPrimary;
        Theme::TextSecondary = DarkTheme::TextSecondary;
        Theme::HoverOverlay  = DarkTheme::HoverOverlay;
        Theme::Card          = DarkTheme::Card;
        Theme::Border        = DarkTheme::Border;
        Theme::Shadow        = DarkTheme::Shadow;
        
        Theme::BorderRadius = 0.0f;
        Theme::BorderWidth = 1.0f;
    }
}

void ThemeSwitcher::setTheme(ThemeType theme) {
    if (currentTheme != theme) {
        currentTheme = theme;
        applyTheme();
        notifyThemeChanged();
    }
}

void ThemeSwitcher::setTheme(const std::string& themeName) {
    ThemeType newTheme = ThemeType::Auto;
    
    if (themeName == "light") {
        newTheme = ThemeType::Light;
    } else if (themeName == "dark") {
        newTheme = ThemeType::Dark;
    } else if (themeName == "system") {
        newTheme = ThemeType::System;
    } else if (themeName == "minimal") {
        newTheme = ThemeType::Minimal;
    } else if (themeName == "auto") {
        newTheme = ThemeType::Auto;
    }
    
    setTheme(newTheme);
}

std::string ThemeSwitcher::getCurrentThemeName() const {
    switch (currentTheme) {
        case ThemeType::Auto:
            return "auto";
        case ThemeType::Dark:
            return "dark";
        case ThemeType::Light:
            return "light";
        case ThemeType::System:
            return "system";
        case ThemeType::Minimal:
            return "minimal";
        default:
            return "auto";
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
    for (auto& callback : callbacks) {
        callback(currentTheme);
    }
}

} // namespace MochiUI
