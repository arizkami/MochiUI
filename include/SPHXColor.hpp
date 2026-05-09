#pragma once
#include <gui/SkiaDraw.hpp>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>

// Undefine Win32 RGB macro that conflicts with SPHXColor::RGB
#ifdef RGB
#undef RGB
#endif

namespace SphereUI {

// Color utility that wraps SkColor and provides Hex/RGB/Float/HSL factories.
// Implicitly converts to SkColor, so it works anywhere Skia expects a color.
//
// Usage:
//   paint.setColor(SPHXColor::Hex("#FF5733"));
//   paint.setColor(SPHXColor::HSL(210, 0.8f, 0.55f));
//   paint.setColor(SPHXColor::Float(1.0f, 0.34f, 0.20f));
//   paint.setColor(SPHXColor::RGB(255, 87, 51));
//   paint.setColor(SPHXColor::Hex("#FF5733").darker(0.15f));
class SPHXColor {
public:
    // ── Construction ──────────────────────────────────────────────────────────

    constexpr SPHXColor() : _color(SK_ColorTRANSPARENT) {}
    constexpr SPHXColor(SkColor c) : _color(c) {}

    // ── Hex factory ───────────────────────────────────────────────────────────

    // Integer: 0xRRGGBB (opaque) or 0xAARRGGBB (with alpha)
    static constexpr SPHXColor Hex(uint32_t hex) {
        return SPHXColor(hex > 0x00FFFFFFu
            ? static_cast<SkColor>(hex)
            : (0xFF000000u | hex));
    }

    // String: "#RGB", "#RGBA", "#RRGGBB", "#RRGGBBAA"  (# is optional)
    // Alpha is always the LAST component in string form (CSS convention).
    static SPHXColor Hex(const std::string& hex) { return parseHexStr(hex.c_str(), hex.size()); }
    static SPHXColor Hex(const char* hex)         { return parseHexStr(hex, strlen(hex)); }

    // Parse any CSS color: hex (#RGB/#RRGGBB), named (white/black/red…),
    // rgb(r,g,b), rgba(r,g,b,a), hsl(h,s%,l%), or decimal integer string.
    static SPHXColor parse(const std::string& s) { return parseColorStr(s.c_str(), s.size()); }
    static SPHXColor parse(const char* s)         { return parseColorStr(s, s ? strlen(s) : 0); }

    // ── RGB factory (0-255) ───────────────────────────────────────────────────

    static constexpr SPHXColor RGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        return SPHXColor(SkColorSetARGB(a, r, g, b));
    }

    // ── Float factory (0.0 – 1.0) ─────────────────────────────────────────────

    static SPHXColor Float(float r, float g, float b, float a = 1.0f) {
        return RGB(toU8(r), toU8(g), toU8(b), toU8(a));
    }

    // ── HSL factory ───────────────────────────────────────────────────────────
    // h: 0-360°   s: 0-1   l: 0-1   a: 0-1
    static SPHXColor HSL(float h, float s, float l, float a = 1.0f);

    // ── Named constants ───────────────────────────────────────────────────────

    static constexpr SPHXColor transparent() { return SPHXColor(SK_ColorTRANSPARENT); }
    static constexpr SPHXColor black()       { return SPHXColor(SK_ColorBLACK);       }
    static constexpr SPHXColor white()       { return SPHXColor(SK_ColorWHITE);       }
    static constexpr SPHXColor red()         { return SPHXColor(SK_ColorRED);         }
    static constexpr SPHXColor green()       { return SPHXColor(SK_ColorGREEN);       }
    static constexpr SPHXColor blue()        { return SPHXColor(SK_ColorBLUE);        }
    static constexpr SPHXColor yellow()      { return SPHXColor(SK_ColorYELLOW);      }
    static constexpr SPHXColor cyan()        { return SPHXColor(SK_ColorCYAN);        }
    static constexpr SPHXColor magenta()     { return SPHXColor(SK_ColorMAGENTA);     }

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
    SPHXColor withAlpha(uint8_t alpha) const { return SPHXColor(SkColorSetA(_color, alpha)); }
    SPHXColor withAlpha(float   alpha) const { return withAlpha(toU8(alpha)); }

    // Blend towards white
    SPHXColor lighter(float amount = 0.10f) const {
        float t = clamp01(amount);
        return Float(rf() + (1.0f - rf()) * t,
                     gf() + (1.0f - gf()) * t,
                     bf() + (1.0f - bf()) * t,
                     af());
    }

    // Scale towards black
    SPHXColor darker(float amount = 0.10f) const {
        float s = 1.0f - clamp01(amount);
        return Float(rf() * s, gf() * s, bf() * s, af());
    }

    // Linear interpolation  (t=0 → this, t=1 → other)
    SPHXColor mix(SPHXColor other, float t) const {
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

    constexpr bool operator==(SPHXColor o) const { return _color == o._color; }
    constexpr bool operator!=(SPHXColor o) const { return _color != o._color; }

private:
    SkColor _color;

    static constexpr uint8_t toU8(float f) {
        return static_cast<uint8_t>(f < 0.0f ? 0 : f > 1.0f ? 255 : f * 255.0f + 0.5f);
    }
    static constexpr float clamp01(float f) {
        return f < 0.0f ? 0.0f : f > 1.0f ? 1.0f : f;
    }

    static SPHXColor parseHexStr(const char* s, size_t rawLen);
    static SPHXColor parseColorStr(const char* s, size_t len);
    static SPHXColor parseNamedColor(const char* s, size_t len);
};

// ── Out-of-line definitions ────────────────────────────────────────────────────

inline SPHXColor SPHXColor::HSL(float h, float s, float l, float a) {
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

inline SPHXColor SPHXColor::parseHexStr(const char* s, size_t rawLen) {
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
            return SPHXColor(); // transparent fallback
    }
}

inline SPHXColor SPHXColor::parseNamedColor(const char* s, size_t len) {
    struct Entry { const char* name; uint32_t argb; };
    static const Entry kNames[] = {
        // sorted alphabetically — binary search below
        {"aliceblue",            0xFFF0F8FFu}, {"antiquewhite",         0xFFFAEBD7u},
        {"aqua",                 0xFF00FFFFu}, {"aquamarine",           0xFF7FFFD4u},
        {"azure",                0xFFF0FFFFu}, {"beige",                0xFFF5F5DCu},
        {"bisque",               0xFFFFE4C4u}, {"black",                0xFF000000u},
        {"blanchedalmond",       0xFFFFEBCDu}, {"blue",                 0xFF0000FFu},
        {"blueviolet",           0xFF8A2BE2u}, {"brown",                0xFFA52A2Au},
        {"burlywood",            0xFFDEB887u}, {"cadetblue",            0xFF5F9EA0u},
        {"chartreuse",           0xFF7FFF00u}, {"chocolate",            0xFFD2691Eu},
        {"coral",                0xFFFF7F50u}, {"cornflowerblue",       0xFF6495EDu},
        {"cornsilk",             0xFFFFF8DCu}, {"crimson",              0xFFDC143Cu},
        {"cyan",                 0xFF00FFFFu}, {"darkblue",             0xFF00008Bu},
        {"darkcyan",             0xFF008B8Bu}, {"darkgoldenrod",        0xFFB8860Bu},
        {"darkgray",             0xFFA9A9A9u}, {"darkgreen",            0xFF006400u},
        {"darkgrey",             0xFFA9A9A9u}, {"darkkhaki",            0xFFBDB76Bu},
        {"darkmagenta",          0xFF8B008Bu}, {"darkolivegreen",       0xFF556B2Fu},
        {"darkorange",           0xFFFF8C00u}, {"darkorchid",           0xFF9932CCu},
        {"darkred",              0xFF8B0000u}, {"darksalmon",           0xFFE9967Au},
        {"darkseagreen",         0xFF8FBC8Fu}, {"darkslateblue",        0xFF483D8Bu},
        {"darkslategray",        0xFF2F4F4Fu}, {"darkslategrey",        0xFF2F4F4Fu},
        {"darkturquoise",        0xFF00CED1u}, {"darkviolet",           0xFF9400D3u},
        {"deeppink",             0xFFFF1493u}, {"deepskyblue",          0xFF00BFFFu},
        {"dimgray",              0xFF696969u}, {"dimgrey",              0xFF696969u},
        {"dodgerblue",           0xFF1E90FFu}, {"firebrick",            0xFFB22222u},
        {"floralwhite",          0xFFFFFAF0u}, {"forestgreen",          0xFF228B22u},
        {"fuchsia",              0xFFFF00FFu}, {"gainsboro",            0xFFDCDCDCu},
        {"ghostwhite",           0xFFF8F8FFu}, {"gold",                 0xFFFFD700u},
        {"goldenrod",            0xFFDAA520u}, {"gray",                 0xFF808080u},
        {"green",                0xFF008000u}, {"greenyellow",          0xFFADFF2Fu},
        {"grey",                 0xFF808080u}, {"honeydew",             0xFFF0FFF0u},
        {"hotpink",              0xFFFF69B4u}, {"indianred",            0xFFCD5C5Cu},
        {"indigo",               0xFF4B0082u}, {"ivory",                0xFFFFFFF0u},
        {"khaki",                0xFFF0E68Cu}, {"lavender",             0xFFE6E6FAu},
        {"lavenderblush",        0xFFFFF0F5u}, {"lawngreen",            0xFF7CFC00u},
        {"lemonchiffon",         0xFFFFFACDu}, {"lightblue",            0xFFADD8E6u},
        {"lightcoral",           0xFFF08080u}, {"lightcyan",            0xFFE0FFFFu},
        {"lightgoldenrodyellow", 0xFFFAFAD2u}, {"lightgray",            0xFFD3D3D3u},
        {"lightgreen",           0xFF90EE90u}, {"lightgrey",            0xFFD3D3D3u},
        {"lightpink",            0xFFFFB6C1u}, {"lightsalmon",          0xFFFFA07Au},
        {"lightseagreen",        0xFF20B2AAu}, {"lightskyblue",         0xFF87CEFAu},
        {"lightslategray",       0xFF778899u}, {"lightslategrey",       0xFF778899u},
        {"lightsteelblue",       0xFFB0C4DEu}, {"lightyellow",          0xFFFFFFE0u},
        {"lime",                 0xFF00FF00u}, {"limegreen",            0xFF32CD32u},
        {"linen",                0xFFFAF0E6u}, {"magenta",              0xFFFF00FFu},
        {"maroon",               0xFF800000u}, {"mediumaquamarine",     0xFF66CDAAu},
        {"mediumblue",           0xFF0000CDu}, {"mediumorchid",         0xFFBA55D3u},
        {"mediumpurple",         0xFF9370DBu}, {"mediumseagreen",       0xFF3CB371u},
        {"mediumslateblue",      0xFF7B68EEu}, {"mediumspringgreen",    0xFF00FA9Au},
        {"mediumturquoise",      0xFF48D1CCu}, {"mediumvioletred",      0xFFC71585u},
        {"midnightblue",         0xFF191970u}, {"mintcream",            0xFFF5FFFAu},
        {"mistyrose",            0xFFFFE4E1u}, {"moccasin",             0xFFFFE4B5u},
        {"navajowhite",          0xFFFFDEADu}, {"navy",                 0xFF000080u},
        {"oldlace",              0xFFFDF5E6u}, {"olive",                0xFF808000u},
        {"olivedrab",            0xFF6B8E23u}, {"orange",               0xFFFFA500u},
        {"orangered",            0xFFFF4500u}, {"orchid",               0xFFDA70D6u},
        {"palegoldenrod",        0xFFEEE8AAu}, {"palegreen",            0xFF98FB98u},
        {"paleturquoise",        0xFFAFEEEEu}, {"palevioletred",        0xFFDB7093u},
        {"papayawhip",           0xFFFFEFD5u}, {"peachpuff",            0xFFFFDAB9u},
        {"peru",                 0xFFCD853Fu}, {"pink",                 0xFFFFC0CBu},
        {"plum",                 0xFFDDA0DDu}, {"powderblue",           0xFFB0E0E6u},
        {"purple",               0xFF800080u}, {"rebeccapurple",        0xFF663399u},
        {"red",                  0xFFFF0000u}, {"rosybrown",            0xFFBC8F8Fu},
        {"royalblue",            0xFF4169E1u}, {"saddlebrown",          0xFF8B4513u},
        {"salmon",               0xFFFA8072u}, {"sandybrown",           0xFFF4A460u},
        {"seagreen",             0xFF2E8B57u}, {"seashell",             0xFFFFF5EEu},
        {"sienna",               0xFFA0522Du}, {"silver",               0xFFC0C0C0u},
        {"skyblue",              0xFF87CEEBu}, {"slateblue",            0xFF6A5ACDu},
        {"slategray",            0xFF708090u}, {"slategrey",            0xFF708090u},
        {"snow",                 0xFFFFFAFAu}, {"springgreen",          0xFF00FF7Fu},
        {"steelblue",            0xFF4682B4u}, {"tan",                  0xFFD2B48Cu},
        {"teal",                 0xFF008080u}, {"thistle",              0xFFD8BFD8u},
        {"tomato",               0xFFFF6347u}, {"transparent",          0x00000000u},
        {"turquoise",            0xFF40E0D0u}, {"violet",               0xFFEE82EEu},
        {"wheat",                0xFFF5DEB3u}, {"white",                0xFFFFFFFFu},
        {"whitesmoke",           0xFFF5F5F5u}, {"yellow",               0xFFFFFF00u},
        {"yellowgreen",          0xFF9ACD32u},
    };
    constexpr size_t kCount = sizeof(kNames) / sizeof(kNames[0]);

    // Lowercase copy (max 31 chars)
    char lower[32] = {};
    size_t clen = len < 31 ? len : 31;
    for (size_t i = 0; i < clen; ++i)
        lower[i] = (char)((unsigned char)s[i] | 0x20u);
    lower[clen] = '\0';

    size_t lo = 0, hi = kCount;
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        int cmp = strcmp(lower, kNames[mid].name);
        if      (cmp < 0) hi = mid;
        else if (cmp > 0) lo = mid + 1;
        else return SPHXColor(static_cast<SkColor>(kNames[mid].argb));
    }
    return SPHXColor(); // unknown → transparent
}

inline SPHXColor SPHXColor::parseColorStr(const char* s, size_t len) {
    if (!s || !len) return SPHXColor();

    // trim
    while (len && (*s == ' ' || *s == '\t')) { ++s; --len; }
    while (len && (s[len-1] == ' ' || s[len-1] == '\t')) --len;
    if (!len) return SPHXColor();

    // hex
    if (s[0] == '#') return parseHexStr(s, len);

    // lowercase prefix for function detection
    char pre[5] = {};
    for (int i = 0; i < 4 && (size_t)i < len; ++i)
        pre[i] = (char)((unsigned char)s[i] | 0x20u);

    // rgb(...) / rgba(...)
    if (pre[0]=='r' && pre[1]=='g' && pre[2]=='b') {
        bool hasA = (pre[3] == 'a');
        const char* p = s + 3 + (hasA ? 1 : 0);
        while (*p && *p != '(') ++p;
        if (!*p) return SPHXColor();
        ++p;
        float comps[4] = {0, 0, 0, 1.0f};
        int n = 0;
        while (n < 4 && *p && *p != ')') {
            while (*p == ' ' || *p == '\t') ++p;
            char* end = nullptr;
            float v = strtof(p, &end);
            if (!end || end == p) break;
            p = end;
            while (*p == ' ' || *p == '\t') ++p;
            if (*p == '%') { v /= 100.0f; ++p; }
            comps[n++] = v;
            while (*p == ' ' || *p == '\t') ++p;
            if (*p == ',' || *p == '/') ++p;
        }
        bool is255 = (comps[0] > 1.0f || comps[1] > 1.0f || comps[2] > 1.0f);
        auto to8 = [](float v, bool scale255) -> uint8_t {
            if (scale255) return (uint8_t)(v < 0 ? 0 : v > 255 ? 255 : v + 0.5f);
            return (uint8_t)(v < 0 ? 0 : v > 1 ? 255 : v * 255.0f + 0.5f);
        };
        uint8_t a = (uint8_t)(comps[3] < 0 ? 0 : comps[3] > 1 ? 255 : comps[3] * 255.0f + 0.5f);
        return RGB(to8(comps[0], is255), to8(comps[1], is255), to8(comps[2], is255), a);
    }

    // hsl(...) / hsla(...)
    if (pre[0]=='h' && pre[1]=='s' && pre[2]=='l') {
        bool hasA = (pre[3] == 'a');
        const char* p = s + 3 + (hasA ? 1 : 0);
        while (*p && *p != '(') ++p;
        if (!*p) return SPHXColor();
        ++p;
        float comps[4] = {0, 0, 0, 1.0f};
        int n = 0;
        while (n < 4 && *p && *p != ')') {
            while (*p == ' ' || *p == '\t') ++p;
            char* end = nullptr;
            float v = strtof(p, &end);
            if (!end || end == p) break;
            p = end;
            // skip unit suffix (deg, rad, %, etc.)
            while (*p && *p != ',' && *p != '/' && *p != ')' && *p != ' ') ++p;
            comps[n++] = v;
            while (*p == ' ' || *p == '\t') ++p;
            if (*p == ',' || *p == '/') ++p;
        }
        // comps[1] and [2] are 0-100 (%), [3] is 0-1 alpha
        return HSL(comps[0], comps[1] / 100.0f, comps[2] / 100.0f,
                   comps[3] < 0 ? 0.0f : comps[3] > 1.0f ? 1.0f : comps[3]);
    }

    // decimal integer string (React Native passes 0xAARRGGBB as unsigned decimal)
    bool allDigits = true;
    for (size_t i = 0; i < len; ++i)
        if (s[i] < '0' || s[i] > '9') { allDigits = false; break; }
    if (allDigits) {
        char* end = nullptr;
        unsigned long v = strtoul(s, &end, 10);
        if (end && end != s) return SPHXColor(static_cast<SkColor>(v));
    }

    return parseNamedColor(s, len);
}

} // namespace SphereUI
