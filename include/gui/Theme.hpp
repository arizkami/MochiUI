#pragma once
#include <include/core/SkColor.h>

namespace MochiUI {

// Dark Theme Colors
namespace DarkTheme {
static constexpr SkColor Background    = SkColorSetRGB(20, 20, 20);
static constexpr SkColor Sidebar       = SkColorSetRGB(30, 30, 30);
static constexpr SkColor MenuBar       = SkColorSetRGB(45, 45, 45);
static constexpr SkColor Accent        = SkColorSetRGB(66, 133, 244);
static constexpr SkColor TextPrimary   = SkColorSetRGB(255, 255, 255);
static constexpr SkColor TextSecondary = SkColorSetRGB(180, 180, 180);
static constexpr SkColor HoverOverlay  = SkColorSetARGB(40, 255, 255, 255);
static constexpr SkColor Card          = SkColorSetARGB(100, 80, 80, 80);
} // namespace DarkTheme

// Light Theme Colors
namespace LightTheme {
static constexpr SkColor Background    = SkColorSetRGB(245, 245, 245);
static constexpr SkColor Sidebar       = SkColorSetRGB(230, 230, 230);
static constexpr SkColor MenuBar       = SkColorSetRGB(220, 220, 220);
static constexpr SkColor Accent        = SkColorSetRGB(0, 120, 215);
static constexpr SkColor TextPrimary   = SkColorSetRGB(30, 30, 30);
static constexpr SkColor TextSecondary = SkColorSetRGB(100, 100, 100);
static constexpr SkColor HoverOverlay  = SkColorSetARGB(30, 0, 0, 0);
static constexpr SkColor Card          = SkColorSetARGB(100, 255, 255, 255);
} // namespace LightTheme

// Active Theme - defaults to Dark, can be switched at runtime
namespace Theme {
inline SkColor Background    = DarkTheme::Background;
inline SkColor Sidebar       = DarkTheme::Sidebar;
inline SkColor MenuBar       = DarkTheme::MenuBar;
inline SkColor Accent        = DarkTheme::Accent;
inline SkColor TextPrimary   = DarkTheme::TextPrimary;
inline SkColor TextSecondary = DarkTheme::TextSecondary;
inline SkColor HoverOverlay  = DarkTheme::HoverOverlay;
inline SkColor Card          = DarkTheme::Card;
} // namespace Theme

} // namespace MochiUI
