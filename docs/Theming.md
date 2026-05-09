# Theming in SphereUI

SphereUI now ships with a small built-in theme surface: `Dark` and `Light`.

The concrete color tokens live directly in `include/gui/Theme.hpp`; there is no JSON theme generation step anymore.

## Switching Themes

You can switch the active theme using the `ThemeSwitcher` singleton.

```cpp
#include <utils/Misc/ThemeSwitcher.hpp>

// Switch to Light theme
SphereUI::ThemeSwitcher::getInstance().setTheme(SphereUI::ThemeType::Light);
```

## Using Theme Tokens

When building custom components, it is recommended to use tokens from the `Theme` namespace. This ensures your UI stays consistent when the theme changes.

```cpp
#include <gui/Theme.hpp>

using namespace SphereUI;

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
SphereUI::Theme::Accent = SphereUI::SPHXColor::Hex("#FF5733");
```

## Component Styling

Interactive node components such as `SliderNode`, `SwitchNode`, and `KnobNode` expose `setStyleOverrides(...)` helpers for per-instance visual customization while still inheriting sensible theme-based defaults.
