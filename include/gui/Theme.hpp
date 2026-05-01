#pragma once
#include <include/core/SkColor.h>

namespace AureliaUI {

// Dark Theme
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

// Light Theme
namespace LightTheme {
static constexpr SkColor Background    = SkColorSetRGB(245, 245, 245);
static constexpr SkColor Sidebar       = SkColorSetRGB(230, 230, 230);
static constexpr SkColor MenuBar       = SkColorSetRGB(220, 220, 220);
static constexpr SkColor Accent        = SkColorSetRGB(0, 120, 215);
static constexpr SkColor TextPrimary   = SkColorSetRGB(30, 30, 30);
static constexpr SkColor TextSecondary = SkColorSetRGB(100, 100, 100);
static constexpr SkColor HoverOverlay  = SkColorSetARGB(30, 0, 0, 0);
static constexpr SkColor Card          = SkColorSetARGB(100, 255, 255, 255);
static constexpr SkColor Border        = SkColorSetRGB(50, 50, 50);
static constexpr SkColor Shadow        = SkColorSetARGB(0, 0, 0, 100);
} // namespace LightTheme

// Md3dark Theme
namespace Md3darkTheme {
static constexpr SkColor Background    = SkColorSetRGB(26, 27, 30);
static constexpr SkColor Sidebar       = SkColorSetRGB(33, 34, 38);
static constexpr SkColor MenuBar       = SkColorSetRGB(43, 44, 48);
static constexpr SkColor Accent        = SkColorSetRGB(208, 188, 255);
static constexpr SkColor TextPrimary   = SkColorSetRGB(230, 225, 229);
static constexpr SkColor TextSecondary = SkColorSetRGB(147, 143, 153);
static constexpr SkColor HoverOverlay  = SkColorSetARGB(35, 255, 255, 255);
static constexpr SkColor Card          = SkColorSetARGB(100, 73, 69, 79);
static constexpr SkColor Border        = SkColorSetRGB(50, 50, 50);
static constexpr SkColor Shadow        = SkColorSetARGB(0, 0, 0, 100);
} // namespace Md3darkTheme

// Md3light Theme
namespace Md3lightTheme {
static constexpr SkColor Background    = SkColorSetRGB(254, 247, 255);
static constexpr SkColor Sidebar       = SkColorSetRGB(243, 237, 247);
static constexpr SkColor MenuBar       = SkColorSetRGB(232, 222, 248);
static constexpr SkColor Accent        = SkColorSetRGB(103, 80, 164);
static constexpr SkColor TextPrimary   = SkColorSetRGB(29, 27, 32);
static constexpr SkColor TextSecondary = SkColorSetRGB(73, 69, 79);
static constexpr SkColor HoverOverlay  = SkColorSetARGB(20, 103, 80, 164);
static constexpr SkColor Card          = SkColorSetARGB(100, 255, 255, 255);
static constexpr SkColor Border        = SkColorSetRGB(50, 50, 50);
static constexpr SkColor Shadow        = SkColorSetARGB(0, 0, 0, 100);
} // namespace Md3lightTheme

// Minimal Theme
namespace MinimalTheme {
static constexpr SkColor Background    = SkColorSetRGB(245, 245, 245);
static constexpr SkColor Sidebar       = SkColorSetRGB(230, 230, 230);
static constexpr SkColor MenuBar       = SkColorSetRGB(220, 220, 220);
static constexpr SkColor Accent        = SkColorSetRGB(0, 120, 215);
static constexpr SkColor TextPrimary   = SkColorSetRGB(30, 30, 30);
static constexpr SkColor TextSecondary = SkColorSetRGB(100, 100, 100);
static constexpr SkColor HoverOverlay  = SkColorSetARGB(30, 0, 0, 0);
static constexpr SkColor Card          = SkColorSetARGB(100, 255, 255, 255);
static constexpr SkColor Border        = SkColorSetRGB(50, 50, 50);
static constexpr SkColor Shadow        = SkColorSetARGB(0, 0, 0, 100);
} // namespace MinimalTheme

// System Theme
namespace SystemTheme {
static constexpr SkColor Background    = SkColorSetRGB(32, 32, 32);
static constexpr SkColor Sidebar       = SkColorSetRGB(40, 40, 40);
static constexpr SkColor MenuBar       = SkColorSetRGB(45, 45, 45);
static constexpr SkColor Accent        = SkColorSetRGB(0, 165, 224);
static constexpr SkColor TextPrimary   = SkColorSetRGB(255, 255, 255);
static constexpr SkColor TextSecondary = SkColorSetRGB(160, 160, 160);
static constexpr SkColor HoverOverlay  = SkColorSetARGB(25, 255, 255, 255);
static constexpr SkColor Card          = SkColorSetARGB(120, 60, 60, 60);
static constexpr SkColor Border        = SkColorSetRGB(50, 50, 50);
static constexpr SkColor Shadow        = SkColorSetARGB(0, 0, 0, 100);
} // namespace SystemTheme

// Winuidark Theme
namespace WinuidarkTheme {
static constexpr SkColor Background    = SkColorSetRGB(32, 32, 32);
static constexpr SkColor Sidebar       = SkColorSetRGB(40, 40, 40);
static constexpr SkColor MenuBar       = SkColorSetRGB(45, 45, 45);
static constexpr SkColor Accent        = SkColorSetRGB(0, 165, 224);
static constexpr SkColor TextPrimary   = SkColorSetRGB(255, 255, 255);
static constexpr SkColor TextSecondary = SkColorSetRGB(160, 160, 160);
static constexpr SkColor HoverOverlay  = SkColorSetARGB(25, 255, 255, 255);
static constexpr SkColor Card          = SkColorSetARGB(120, 60, 60, 60);
static constexpr SkColor Border        = SkColorSetRGB(50, 50, 50);
static constexpr SkColor Shadow        = SkColorSetARGB(0, 0, 0, 100);
} // namespace WinuidarkTheme

// Winuilight Theme
namespace WinuilightTheme {
static constexpr SkColor Background    = SkColorSetRGB(243, 243, 243);
static constexpr SkColor Sidebar       = SkColorSetRGB(238, 238, 238);
static constexpr SkColor MenuBar       = SkColorSetRGB(249, 249, 249);
static constexpr SkColor Accent        = SkColorSetRGB(0, 90, 158);
static constexpr SkColor TextPrimary   = SkColorSetRGB(0, 0, 0);
static constexpr SkColor TextSecondary = SkColorSetRGB(102, 102, 102);
static constexpr SkColor HoverOverlay  = SkColorSetARGB(15, 0, 0, 0);
static constexpr SkColor Card          = SkColorSetARGB(200, 255, 255, 255);
static constexpr SkColor Border        = SkColorSetRGB(50, 50, 50);
static constexpr SkColor Shadow        = SkColorSetARGB(0, 0, 0, 100);
} // namespace WinuilightTheme

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

// Non-color theme properties
inline float BorderWidth     = 1.0f;
inline float BorderRadius    = 4.0f;
inline float ControlHeight   = 32.0f;
inline float FontSmall       = 12.0f;
inline float FontNormal      = 14.0f;
inline float FontMedium      = 16.0f;
inline float FontLarge       = 18.0f;
inline float FontHeader      = 24.0f;
} // namespace Theme

} // namespace AureliaUI