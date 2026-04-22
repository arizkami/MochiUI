#include <include/utils/FontManager/FontMgr.hpp>
#include <windows.h>
#include <include/ports/SkTypeface_win.h>
#include <vector>

namespace MochiUI {

namespace {
    struct Run {
        sk_sp<SkTypeface> typeface;
        std::string text;
    };

    uint32_t nextUTF8(const char** ptr, const char* end) {
        const uint8_t* p = (const uint8_t*)*ptr;
        if (p >= (const uint8_t*)end) return 0;
        uint32_t c = *p++;
        if (c < 0x80) {
            *ptr = (const char*)p;
            return c;
        } else if ((c >> 5) == 0x06) {
            if (p >= (const uint8_t*)end) return 0;
            c = ((c & 0x1F) << 6) | (*p++ & 0x3F);
        } else if ((c >> 4) == 0x0E) {
            if (p + 1 >= (const uint8_t*)end) return 0;
            c = ((c & 0x0F) << 12) | ((p[0] & 0x3F) << 6) | (p[1] & 0x3F);
            p += 2;
        } else if ((c >> 3) == 0x1E) {
            if (p + 2 >= (const uint8_t*)end) return 0;
            c = ((c & 0x07) << 18) | ((p[0] & 0x3F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
            p += 3;
        } else {
            *ptr = (const char*)p;
            return 0;
        }
        *ptr = (const char*)p;
        return c;
    }

    std::vector<Run> shapeText(const std::string& text, SkFontMgr* mgr, const std::string& defaultFamily) {
        std::vector<Run> runs;
        if (text.empty() || !mgr) return runs;

        sk_sp<SkTypeface> defaultTypeface = mgr->matchFamilyStyle(defaultFamily.c_str(), SkFontStyle());
        if (!defaultTypeface) {
            defaultTypeface = mgr->legacyMakeTypeface(nullptr, SkFontStyle());
        }

        sk_sp<SkTypeface> currentTypeface = nullptr;
        std::string currentText;

        const char* ptr = text.c_str();
        const char* end = ptr + text.size();

        static const std::vector<std::string> fallbackFonts = {
            "Segoe UI",
            "Microsoft YaHei",     // Simplified Chinese
            "Microsoft JhengHei",  // Traditional Chinese
            "Yu Gothic UI",        // Japanese
            "Malgun Gothic",       // Korean
            "Leelawadee UI",       // Thai
            "Nirmala UI",          // Indic
            "Gadugi",              // Other Indic
            "Segoe UI Emoji",      // Emojis
            "Segoe UI Symbol"      // Symbols
        };

        while (ptr < end) {
            const char* charStart = ptr;
            uint32_t unichar = nextUTF8(&ptr, end);
            if (charStart == ptr) break; // Infinite loop protection

            sk_sp<SkTypeface> tf = defaultTypeface;
            if (defaultTypeface && defaultTypeface->unicharToGlyph(unichar) == 0 && unichar > 32) {
                sk_sp<SkTypeface> fallback = nullptr;
                
                // Try our curated list of modern UI fonts first
                for (const auto& fallbackName : fallbackFonts) {
                    auto candidate = mgr->matchFamilyStyle(fallbackName.c_str(), SkFontStyle());
                    if (candidate && candidate->unicharToGlyph(unichar) != 0) {
                        fallback = candidate;
                        break;
                    }
                }
                
                // If not found in curated list, ask the OS
                if (!fallback) {
                    fallback = mgr->matchFamilyStyleCharacter(
                        defaultFamily.c_str(), SkFontStyle(), nullptr, 0, unichar);
                }
                
                if (fallback) {
                    tf = fallback;
                }
            }

            if (currentTypeface && currentTypeface.get() != tf.get()) {
                runs.push_back({currentTypeface, currentText});
                currentText.clear();
            }
            currentTypeface = tf;
            currentText.append(charStart, ptr - charStart);
        }
        if (currentTypeface && !currentText.empty()) {
            runs.push_back({currentTypeface, currentText});
        }
        return runs;
    }
}

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

float FontManager::drawText(SkCanvas* canvas, const std::string& text, float x, float y, float fontSize, const SkPaint& paint, const std::string& familyName) {
    if (!fFontMgr) initialize();
    auto runs = shapeText(text, fFontMgr.get(), familyName);
    float currentX = x;
    for (const auto& run : runs) {
        SkFont font(run.typeface, fontSize);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setSubpixel(true);
        canvas->drawSimpleText(run.text.c_str(), run.text.size(), SkTextEncoding::kUTF8, currentX, y, font, paint);
        currentX += font.measureText(run.text.c_str(), run.text.size(), SkTextEncoding::kUTF8);
    }
    return currentX - x;
}

float FontManager::measureText(const std::string& text, size_t byteLength, float fontSize, SkRect* outBounds, const std::string& familyName) {
    if (!fFontMgr) initialize();
    std::string subText = text.substr(0, byteLength);
    auto runs = shapeText(subText, fFontMgr.get(), familyName);
    
    float totalWidth = 0.0f;
    SkRect totalBounds = SkRect::MakeEmpty();
    
    for (const auto& run : runs) {
        SkFont font(run.typeface, fontSize);
        SkRect runBounds;
        float runWidth = font.measureText(run.text.c_str(), run.text.size(), SkTextEncoding::kUTF8, outBounds ? &runBounds : nullptr);
        
        if (outBounds) {
            runBounds.offset(totalWidth, 0);
            if (totalBounds.isEmpty()) {
                totalBounds = runBounds;
            } else {
                totalBounds.join(runBounds);
            }
        }
        totalWidth += runWidth;
    }
    
    if (outBounds) {
        *outBounds = totalBounds;
    }
    return totalWidth;
}

float FontManager::measureText(const std::string& text, float fontSize, SkRect* outBounds, const std::string& familyName) {
    return measureText(text, text.size(), fontSize, outBounds, familyName);
}

} // namespace MochiUI
