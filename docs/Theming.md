# Theming in AureliaUI

AureliaUI features a flexible theme system that allows you to easily switch between different visual styles at runtime.

## Built-in Themes

AureliaUI comes with several built-in themes:

- `Dark`: A modern dark theme (default).
- `Light`: A clean light theme.
- `Md3Dark` / `Md3Light`: Material Design 3 inspired themes.
- `Minimal`: A simplified, high-contrast theme.
- `WinuiDark` / `WinuiLight`: Windows UI (WinUI) inspired themes.
- `System`: Follows the operating system's theme preference.

## Switching Themes

You can switch the active theme using the `ThemeSwitcher` singleton.

```cpp
#include <utils/Misc/ThemeSwitcher.hpp>

// Switch to Light theme
AureliaUI::ThemeSwitcher::getInstance().setTheme(AureliaUI::ThemeType::Light);
```

## Using Theme Tokens

When building custom components, it is recommended to use tokens from the `Theme` namespace. This ensures your UI stays consistent when the theme changes.

```cpp
#include <gui/Theme.hpp>

using namespace AureliaUI;

auto text = std::make_shared<TextNode>("Hello");
text->color = Theme::TextPrimary; // Automatically uses current theme's primary text color
```

## Common Theme Tokens

- `Theme::Background`: Main application background.
- `Theme::Sidebar`: Sidebar background.
- `Theme::Accent`: The primary brand/action color.
- `Theme::TextPrimary`: Color for main text.
- `Theme::TextSecondary`: Color for secondary/muted text.
- `Theme::Card`: Background for cards and containers.
- `Theme::Border`: Default border color.

## Dynamic Customization

You can also modify theme tokens directly at runtime for global changes:

```cpp
AureliaUI::Theme::Accent = AureliaUI::AUKColor::Hex("#FF5733");
```
