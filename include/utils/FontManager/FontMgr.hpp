#pragma once
#include <gui/SkiaDraw.hpp>
#include <string>
#include <memory>
#include <map>

namespace SphereUI {

class FontManager {
public:
    static FontManager& getInstance();

    void initialize();

    sk_sp<SkTypeface> getTypeface(const std::string& familyName,
                                   SkFontStyle style = SkFontStyle());

    SkFont createFont(const std::string& familyName,
                      float size,
                      SkFontStyle style = SkFontStyle());

    float drawText(SkCanvas* canvas, const std::string& text, float x, float y, float fontSize, const SkPaint& paint, const std::string& familyName = DEFAULT_FONT);
    float measureText(const std::string& text, float fontSize, SkRect* outBounds = nullptr, const std::string& familyName = DEFAULT_FONT);
    float measureText(const std::string& text, size_t byteLength, float fontSize, SkRect* outBounds = nullptr, const std::string& familyName = DEFAULT_FONT);

    void getFontMetrics(float fontSize, SkFontMetrics* metrics, const std::string& familyName = DEFAULT_FONT);

    sk_sp<SkFontMgr> getFontMgr() const { return fFontMgr; }

    static constexpr const char* DEFAULT_FONT = "Segoe UI";
    static constexpr const char* MONOSPACE_FONT = "Consolas";
    static constexpr const char* EMOJI_FONT = "Segoe UI Emoji";

private:
    FontManager() = default;
    ~FontManager() = default;
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    sk_sp<SkFontMgr> fFontMgr;
    std::map<std::string, sk_sp<SkTypeface>> fTypefaceCache;
};

} // namespace SphereUI
