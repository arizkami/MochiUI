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
static constexpr SkColor Border        = SkColorSetRGB(50, 50, 50);
static constexpr SkColor Shadow        = SkColorSetARGB(0, 0, 0, 100);
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
static constexpr SkColor Border        = SkColorSetRGB(200, 200, 200);
static constexpr SkColor Shadow        = SkColorSetARGB(0, 0, 0, 40);
} // namespace LightTheme

// System Theme (Classic Windows/System look)
namespace SystemTheme {
static constexpr SkColor Background    = SkColorSetRGB(240, 240, 240);
static constexpr SkColor Sidebar       = SkColorSetRGB(240, 240, 240);
static constexpr SkColor MenuBar       = SkColorSetRGB(240, 240, 240);
static constexpr SkColor Accent        = SkColorSetRGB(0, 120, 215);
static constexpr SkColor TextPrimary   = SkColorSetRGB(0, 0, 0);
static constexpr SkColor TextSecondary = SkColorSetRGB(100, 100, 100);
static constexpr SkColor HoverOverlay  = SkColorSetARGB(30, 0, 0, 0);
static constexpr SkColor Card          = SkColorSetRGB(255, 255, 255);
static constexpr SkColor Border        = SkColorSetRGB(160, 160, 160);
static constexpr SkColor Shadow        = SkColorSetARGB(0, 0, 0, 50);
} // namespace SystemTheme

// Minimal Theme (Modern, high contrast, clean)
namespace MinimalTheme {
static constexpr SkColor Background    = SkColorSetRGB(255, 255, 255);
static constexpr SkColor Sidebar       = SkColorSetRGB(250, 250, 250);
static constexpr SkColor MenuBar       = SkColorSetRGB(255, 255, 255);
static constexpr SkColor Accent        = SkColorSetRGB(0, 0, 0);
static constexpr SkColor TextPrimary   = SkColorSetRGB(0, 0, 0);
static constexpr SkColor TextSecondary = SkColorSetRGB(120, 120, 120);
static constexpr SkColor HoverOverlay  = SkColorSetARGB(15, 0, 0, 0);
static constexpr SkColor Card          = SkColorSetRGB(255, 255, 255);
static constexpr SkColor Border        = SkColorSetRGB(230, 230, 230);
static constexpr SkColor Shadow        = SkColorSetARGB(0, 0, 0, 20);
} // namespace MinimalTheme

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
inline SkColor Border        = DarkTheme::Border;
inline SkColor Shadow        = DarkTheme::Shadow;

// Visual Style Properties (UI/UX metrics)
inline float BorderRadius = 0.0f; // Default to flat/no rounding
inline float BorderWidth = 1.0f;
inline float ControlPadding = 8.0f;
inline float ControlHeight = 32.0f; // Default height for inputs/buttons

// Standard Font Scale
inline float FontSmall  = 12.0f;
inline float FontNormal = 13.0f;
inline float FontMedium = 16.0f;
inline float FontLarge  = 24.0f;
inline float FontHeader = 42.0f;
inline float FontHero   = 60.0f;

inline float FontSize = FontNormal;
} // namespace Theme

} // namespace MochiUI
