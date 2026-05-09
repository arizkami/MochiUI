#include <utils/Misc/ThemeSwitcher.hpp>
#include <gui/Theme.hpp>

namespace SphereUI {

ThemeSwitcher& ThemeSwitcher::getInstance() {
    static ThemeSwitcher instance;
    return instance;
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
    if (currentTheme == ThemeType::Light) {
        APPLY_THEME(LightTheme);
    } else {
        APPLY_THEME(DarkTheme);
    }

    Theme::BorderRadius = 4.0f;
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
    ThemeType t = ThemeType::Dark;
    if (themeName == "light") t = ThemeType::Light;
    setTheme(t);
}

std::string ThemeSwitcher::getCurrentThemeName() const {
    switch (currentTheme) {
        case ThemeType::Dark:  return "dark";
        case ThemeType::Light: return "light";
    }
    return "dark";
}

void ThemeSwitcher::registerThemeChangeCallback(std::function<void(ThemeType)> callback) {
    callbacks.push_back(callback);
}

void ThemeSwitcher::notifyThemeChanged() {
    for (auto& cb : callbacks) cb(currentTheme);
}

} // namespace SphereUI
