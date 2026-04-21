#include <include/utils/FontManager/FontMgr.hpp>
#include <windows.h>
#include <include/ports/SkTypeface_win.h>

namespace MochiUI {

FontManager& FontManager::getInstance() {
    static FontManager instance;
    return instance;
}

void FontManager::initialize() {
    if (!fFontMgr) {
        fFontMgr = SkFontMgr_New_DirectWrite();
    }
}

sk_sp<SkTypeface> FontManager::getTypeface(const std::string& familyName, 
                                            SkFontStyle style) {
    if (!fFontMgr) {
        initialize();
    }
    
    std::string cacheKey = familyName + "_" + 
                          std::to_string(style.weight()) + "_" +
                          std::to_string(style.width()) + "_" +
                          std::to_string(style.slant());
    
    auto it = fTypefaceCache.find(cacheKey);
    if (it != fTypefaceCache.end()) {
        return it->second;
    }
    
    sk_sp<SkTypeface> typeface = fFontMgr->matchFamilyStyle(familyName.c_str(), style);
    
    if (typeface) {
        fTypefaceCache[cacheKey] = typeface;
    }
    
    return typeface;
}

SkFont FontManager::createFont(const std::string& familyName, 
                                float size, 
                                SkFontStyle style) {
    auto typeface = getTypeface(familyName, style);
    SkFont font(typeface, size);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
    font.setSubpixel(true);
    return font;
}

} // namespace MochiUI
