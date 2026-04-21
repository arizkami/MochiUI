#pragma once
#include <include/core/SkFontMgr.h>
#include <include/core/SkTypeface.h>
#include <include/core/SkFont.h>
#include <string>
#include <memory>
#include <map>

namespace MochiUI {

class FontManager {
public:
    static FontManager& getInstance();
    
    void initialize();
    
    sk_sp<SkTypeface> getTypeface(const std::string& familyName, 
                                   SkFontStyle style = SkFontStyle());
    
    SkFont createFont(const std::string& familyName, 
                      float size, 
                      SkFontStyle style = SkFontStyle());
    
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

} // namespace MochiUI
