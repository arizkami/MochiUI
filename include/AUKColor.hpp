#pragma once
#include <gui/SkiaDraw.hpp>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>

// Undefine Win32 RGB macro that conflicts with AUKColor::RGB
#ifdef RGB
#undef RGB
#endif

namespace AureliaUI {

// Color utility that wraps SkColor and provides Hex/RGB/Float/HSL factories.
// Implicitly converts to SkColor, so it works anywhere Skia expects a color.
//
// Usage:
//   paint.setColor(AUKColor::Hex("#FF5733"));
//   paint.setColor(AUKColor::HSL(210, 0.8f, 0.55f));
//   paint.setColor(AUKColor::Float(1.0f, 0.34f, 0.20f));
//   paint.setColor(AUKColor::RGB(255, 87, 51));
//   paint.setColor(AUKColor::Hex("#FF5733").darker(0.15f));
class AUKColor {
public:
    // ── Construction ──────────────────────────────────────────────────────────

    constexpr AUKColor() : _color(SK_ColorTRANSPARENT) {}
    constexpr AUKColor(SkColor c) : _color(c) {}

    // ── Hex factory ───────────────────────────────────────────────────────────

    // Integer: 0xRRGGBB (opaque) or 0xAARRGGBB (with alpha)
    static constexpr AUKColor Hex(uint32_t hex) {
        return AUKColor(hex > 0x00FFFFFFu
            ? static_cast<SkColor>(hex)
            : (0xFF000000u | hex));
    }

    // String: "#RGB", "#RGBA", "#RRGGBB", "#RRGGBBAA"  (# is optional)
    // Alpha is always the LAST component in string form (CSS convention).
    static AUKColor Hex(const std::string& hex) { return parseHexStr(hex.c_str(), hex.size()); }
    static AUKColor Hex(const char* hex)         { return parseHexStr(hex, strlen(hex)); }

    // ── RGB factory (0-255) ───────────────────────────────────────────────────

    static constexpr AUKColor RGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        return AUKColor(SkColorSetARGB(a, r, g, b));
    }

    // ── Float factory (0.0 – 1.0) ─────────────────────────────────────────────

    static AUKColor Float(float r, float g, float b, float a = 1.0f) {
        return RGB(toU8(r), toU8(g), toU8(b), toU8(a));
    }

    // ── HSL factory ───────────────────────────────────────────────────────────
    // h: 0-360°   s: 0-1   l: 0-1   a: 0-1
    static AUKColor HSL(float h, float s, float l, float a = 1.0f);

    // ── Named constants ───────────────────────────────────────────────────────

    static constexpr AUKColor transparent() { return AUKColor(SK_ColorTRANSPARENT); }
    static constexpr AUKColor black()       { return AUKColor(SK_ColorBLACK);       }
    static constexpr AUKColor white()       { return AUKColor(SK_ColorWHITE);       }
    static constexpr AUKColor red()         { return AUKColor(SK_ColorRED);         }
    static constexpr AUKColor green()       { return AUKColor(SK_ColorGREEN);       }
    static constexpr AUKColor blue()        { return AUKColor(SK_ColorBLUE);        }
    static constexpr AUKColor yellow()      { return AUKColor(SK_ColorYELLOW);      }
    static constexpr AUKColor cyan()        { return AUKColor(SK_ColorCYAN);        }
    static constexpr AUKColor magenta()     { return AUKColor(SK_ColorMAGENTA);     }

    // ── Implicit conversion ───────────────────────────────────────────────────

    constexpr operator SkColor() const { return _color; }
    constexpr SkColor toSkColor()      const { return _color; }

    // ── Component accessors ───────────────────────────────────────────────────

    uint8_t r() const { return SkColorGetR(_color); }
    uint8_t g() const { return SkColorGetG(_color); }
    uint8_t b() const { return SkColorGetB(_color); }
    uint8_t a() const { return SkColorGetA(_color); }

    float rf() const { return r() / 255.0f; }
    float gf() const { return g() / 255.0f; }
    float bf() const { return b() / 255.0f; }
    float af() const { return a() / 255.0f; }

    // ── Manipulation ──────────────────────────────────────────────────────────

    // Replace the alpha channel (preserves RGB)
    AUKColor withAlpha(uint8_t alpha) const { return AUKColor(SkColorSetA(_color, alpha)); }
    AUKColor withAlpha(float   alpha) const { return withAlpha(toU8(alpha)); }

    // Blend towards white
    AUKColor lighter(float amount = 0.10f) const {
        float t = clamp01(amount);
        return Float(rf() + (1.0f - rf()) * t,
                     gf() + (1.0f - gf()) * t,
                     bf() + (1.0f - bf()) * t,
                     af());
    }

    // Scale towards black
    AUKColor darker(float amount = 0.10f) const {
        float s = 1.0f - clamp01(amount);
        return Float(rf() * s, gf() * s, bf() * s, af());
    }

    // Linear interpolation  (t=0 → this, t=1 → other)
    AUKColor mix(AUKColor other, float t) const {
        t = clamp01(t);
        return Float(rf() + (other.rf() - rf()) * t,
                     gf() + (other.gf() - gf()) * t,
                     bf() + (other.bf() - bf()) * t,
                     af() + (other.af() - af()) * t);
    }

    // ── String output ─────────────────────────────────────────────────────────

    // Returns "#RRGGBB" (includeAlpha=false) or "#RRGGBBAA" (includeAlpha=true)
    std::string toHexString(bool includeAlpha = false) const {
        char buf[10];
        if (includeAlpha)
            snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", r(), g(), b(), a());
        else
            snprintf(buf, sizeof(buf), "#%02X%02X%02X", r(), g(), b());
        return buf;
    }

    // ── Comparison ────────────────────────────────────────────────────────────

    constexpr bool operator==(AUKColor o) const { return _color == o._color; }
    constexpr bool operator!=(AUKColor o) const { return _color != o._color; }

private:
    SkColor _color;

    static constexpr uint8_t toU8(float f) {
        return static_cast<uint8_t>(f < 0.0f ? 0 : f > 1.0f ? 255 : f * 255.0f + 0.5f);
    }
    static constexpr float clamp01(float f) {
        return f < 0.0f ? 0.0f : f > 1.0f ? 1.0f : f;
    }

    static AUKColor parseHexStr(const char* s, size_t rawLen);
};

// ── Out-of-line definitions ────────────────────────────────────────────────────

inline AUKColor AUKColor::HSL(float h, float s, float l, float a) {
    h = std::fmod(h, 360.0f);
    if (h < 0.0f) h += 360.0f;
    s = clamp01(s);
    l = clamp01(l);
    a = clamp01(a);

    if (s == 0.0f) return Float(l, l, l, a); // achromatic — grey

    // Standard HSL → RGB conversion
    auto hue2rgb = [](float p, float q, float t) -> float {
        if (t < 0.0f) t += 1.0f;
        if (t > 1.0f) t -= 1.0f;
        if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
        if (t < 0.5f)         return q;
        if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
        return p;
    };

    float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
    float p = 2.0f * l - q;
    float hh = h / 360.0f;

    return Float(hue2rgb(p, q, hh + 1.0f / 3.0f),
                 hue2rgb(p, q, hh),
                 hue2rgb(p, q, hh - 1.0f / 3.0f),
                 a);
}

inline AUKColor AUKColor::parseHexStr(const char* s, size_t rawLen) {
    if (rawLen > 0 && s[0] == '#') { ++s; --rawLen; }

    auto nibble = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
        if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
        if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
        return 0;
    };
    auto byte2 = [&](const char* p) -> uint8_t {
        return static_cast<uint8_t>((nibble(p[0]) << 4) | nibble(p[1]));
    };
    auto expand = [&](char c) -> uint8_t {
        uint8_t n = nibble(c);
        return static_cast<uint8_t>((n << 4) | n);
    };

    switch (rawLen) {
        case 3:  // #RGB  → RRGGBB, opaque
            return RGB(expand(s[0]), expand(s[1]), expand(s[2]));
        case 4:  // #RGBA → RRGGBBAA
            return RGB(expand(s[0]), expand(s[1]), expand(s[2]), expand(s[3]));
        case 6:  // #RRGGBB, opaque
            return RGB(byte2(s), byte2(s+2), byte2(s+4));
        case 8:  // #RRGGBBAA
            return RGB(byte2(s), byte2(s+2), byte2(s+4), byte2(s+6));
        default:
            return AUKColor(); // transparent fallback
    }
}

} // namespace AureliaUI
