#include <dsl/LibCssStyle.hpp>

// MSVC C++ does not accept C99 `restrict` in declarations the way libcss headers use it.
#if defined(_MSC_VER)
#pragma push_macro("restrict")
#undef restrict
#define restrict __restrict
#endif

// Include libcss before any Skia/Windows headers: the SDK defines macros such as
// `opacity` that corrupt libcss/computed.h parameter names in C++.
#include <libcss/libcss.h>
#include <libcss/computed.h>
#include <libcss/fpmath.h>
#include <libcss/unit.h>

#if defined(_MSC_VER)
#pragma pop_macro("restrict")
#endif

#include <dsl/DslInherited.hpp>
#include <AUKDSL.hpp>

#include <cstring>
#include <mutex>
#include <string>

namespace AureliaUI {

namespace {

#define UNUSED(x) (void)(x)

static float fixToFloat(css_fixed f) { return FIXTOFLT(f); }

static float lenToPx(const css_computed_style* style, const css_unit_ctx* ctx,
                       css_fixed len, css_unit unit) {
    return fixToFloat(css_unit_len2css_px(style, ctx, len, unit));
}

static std::string lwcStr(lwc_string* s) {
    if (!s) return {};
    return std::string(reinterpret_cast<const char*>(lwc_string_data(s)),
                       lwc_string_length(s));
}

static css_error resolve_url(void* pw, const char* base, lwc_string* rel, lwc_string** abs) {
    UNUSED(pw);
    UNUSED(base);
    *abs = lwc_string_ref(rel);
    return CSS_OK;
}

static css_error ua_default_for_property(void* pw, uint32_t property, css_hint* hint) {
    UNUSED(pw);
    if (property == CSS_PROP_COLOR) {
        hint->data.color = 0xff000000u;
        hint->status = CSS_COLOR_COLOR;
    } else if (property == CSS_PROP_FONT_FAMILY) {
        hint->data.strings = nullptr;
        hint->status = CSS_FONT_FAMILY_SANS_SERIF;
    } else if (property == CSS_PROP_QUOTES) {
        hint->data.strings = nullptr;
        hint->status = CSS_QUOTES_NONE;
    } else if (property == CSS_PROP_VOICE_FAMILY) {
        hint->data.strings = nullptr;
        hint->status = 0;
    } else {
        return CSS_INVALID;
    }
    return CSS_OK;
}

static css_error set_libcss_node_data(void* pw, void* n, void* libcss_node_data);

static css_error get_libcss_node_data(void* pw, void* n, void** libcss_node_data) {
    UNUSED(pw);
    UNUSED(n);
    *libcss_node_data = nullptr;
    return CSS_OK;
}

static css_error node_name_stub(void* pw, void* node, css_qname* qname) {
    UNUSED(pw);
    lwc_string* tag = static_cast<lwc_string*>(node);
    qname->name = lwc_string_ref(tag);
    return CSS_OK;
}

#define STUB_FALSE(name) \
    static css_error name(void* pw, void* n, bool* m) { \
        UNUSED(pw); \
        UNUSED(n); \
        *m = false; \
        return CSS_OK; \
    }

#define STUB_FALSE_LWC(name) \
    static css_error name(void* pw, void* n, lwc_string* nm, bool* m) { \
        UNUSED(pw); \
        UNUSED(n); \
        UNUSED(nm); \
        *m = false; \
        return CSS_OK; \
    }

#define STUB_FALSE_ATTR_VAL(name) \
    static css_error name(void* pw, void* n, const css_qname* q, lwc_string* value, bool* m) { \
        UNUSED(pw); \
        UNUSED(n); \
        UNUSED(q); \
        UNUSED(value); \
        *m = false; \
        return CSS_OK; \
    }

#define STUB_FALSE_QNAME(name) \
    static css_error name(void* pw, void* n, const css_qname* q, bool* m) { \
        UNUSED(pw); \
        UNUSED(n); \
        UNUSED(q); \
        *m = false; \
        return CSS_OK; \
    }

static css_error node_classes_stub(void* pw, void* n, lwc_string*** classes, uint32_t* n_classes) {
    UNUSED(pw);
    UNUSED(n);
    *classes = nullptr;
    *n_classes = 0;
    return CSS_OK;
}

static css_error node_id_stub(void* pw, void* n, lwc_string** id) {
    UNUSED(pw);
    UNUSED(n);
    *id = nullptr;
    return CSS_OK;
}

#define STUB_NULL2(name) \
    static css_error name(void* pw, void* n, const css_qname* q, void** out) { \
        UNUSED(pw); \
        UNUSED(n); \
        UNUSED(q); \
        *out = nullptr; \
        return CSS_OK; \
    }

STUB_NULL2(named_ancestor_stub)
STUB_NULL2(named_parent_stub)
STUB_NULL2(named_sibling_stub)
STUB_NULL2(named_generic_sibling_stub)

static css_error parent_node_stub(void* pw, void* n, void** p) {
    UNUSED(pw);
    UNUSED(n);
    *p = nullptr;
    return CSS_OK;
}

static css_error sibling_node_stub(void* pw, void* n, void** s) {
    UNUSED(pw);
    UNUSED(n);
    *s = nullptr;
    return CSS_OK;
}

static css_error lwcToCssError(lwc_error e) {
    switch (e) {
        case lwc_error_ok: return CSS_OK;
        case lwc_error_oom: return CSS_NOMEM;
        default: return CSS_BADPARM;
    }
}

static css_error node_has_name_stub(void* pw, void* n, const css_qname* q, bool* match) {
    UNUSED(pw);
    lwc_string* tag = static_cast<lwc_string*>(n);
    return lwcToCssError(lwc_string_caseless_isequal(tag, q->name, match));
}

STUB_FALSE_LWC(node_has_class_stub)
STUB_FALSE_LWC(node_has_id_stub)
STUB_FALSE_QNAME(node_has_attribute_stub)
STUB_FALSE_ATTR_VAL(node_has_attribute_equal_stub)
STUB_FALSE_ATTR_VAL(node_has_attribute_dashmatch_stub)
STUB_FALSE_ATTR_VAL(node_has_attribute_includes_stub)
STUB_FALSE_ATTR_VAL(node_has_attribute_prefix_stub)
STUB_FALSE_ATTR_VAL(node_has_attribute_suffix_stub)
STUB_FALSE_ATTR_VAL(node_has_attribute_substring_stub)
STUB_FALSE(node_is_root_stub)

static css_error node_count_siblings_stub(void* pw, void* n, bool same, bool after, int32_t* c) {
    UNUSED(pw);
    UNUSED(n);
    UNUSED(same);
    UNUSED(after);
    *c = 1;
    return CSS_OK;
}

STUB_FALSE(node_is_empty_stub)
STUB_FALSE(node_is_link_stub)
STUB_FALSE(node_is_visited_stub)
STUB_FALSE(node_is_hover_stub)
STUB_FALSE(node_is_active_stub)
STUB_FALSE(node_is_focus_stub)
STUB_FALSE(node_is_enabled_stub)
STUB_FALSE(node_is_disabled_stub)
STUB_FALSE(node_is_checked_stub)
STUB_FALSE(node_is_target_stub)

static css_error node_is_lang_stub(void* pw, void* n, lwc_string* lang, bool* m) {
    UNUSED(pw);
    UNUSED(n);
    UNUSED(lang);
    *m = false;
    return CSS_OK;
}

static css_error node_presentational_hint_stub(void* pw, void* n, uint32_t* nh, css_hint** h) {
    UNUSED(pw);
    UNUSED(n);
    *nh = 0;
    *h = nullptr;
    return CSS_OK;
}

static css_select_handler kSelectHandler = {
    CSS_SELECT_HANDLER_VERSION_1,
    node_name_stub,
    node_classes_stub,
    node_id_stub,
    named_ancestor_stub,
    named_parent_stub,
    named_sibling_stub,
    named_generic_sibling_stub,
    parent_node_stub,
    sibling_node_stub,
    node_has_name_stub,
    node_has_class_stub,
    node_has_id_stub,
    node_has_attribute_stub,
    node_has_attribute_equal_stub,
    node_has_attribute_dashmatch_stub,
    node_has_attribute_includes_stub,
    node_has_attribute_prefix_stub,
    node_has_attribute_suffix_stub,
    node_has_attribute_substring_stub,
    node_is_root_stub,
    node_count_siblings_stub,
    node_is_empty_stub,
    node_is_link_stub,
    node_is_visited_stub,
    node_is_hover_stub,
    node_is_active_stub,
    node_is_focus_stub,
    node_is_enabled_stub,
    node_is_disabled_stub,
    node_is_checked_stub,
    node_is_target_stub,
    node_is_lang_stub,
    node_presentational_hint_stub,
    ua_default_for_property,
    set_libcss_node_data,
    get_libcss_node_data,
};

static css_error set_libcss_node_data(void* pw, void* n, void* libcss_node_data) {
    return css_libcss_node_data_handler(&kSelectHandler, CSS_NODE_DELETED, pw, n, nullptr,
                                        libcss_node_data);
}

static css_unit_ctx makeUnitCtx() {
    return css_unit_ctx{
        .viewport_width    = INTTOFIX(1920),
        .viewport_height   = INTTOFIX(1080),
        .font_size_default = INTTOFIX(16),
        .font_size_minimum = INTTOFIX(6),
        .device_dpi        = INTTOFIX(96),
        .root_style        = nullptr,
        .pw                = nullptr,
        .measure           = nullptr,
    };
}

static css_media makeMedia() {
    css_media m{};
    m.type = CSS_MEDIA_SCREEN;
    return m;
}

static void mapDisplay(uint8_t disp, DSLNode& node) {
    switch (disp) {
        case CSS_DISPLAY_FLEX:
        case CSS_DISPLAY_INLINE_FLEX:
            node.style.setFlexDirection(YGFlexDirectionRow);
            break;
        case CSS_DISPLAY_BLOCK:
        case CSS_DISPLAY_INLINE_BLOCK:
            node.style.setFlexDirection(YGFlexDirectionColumn);
            break;
        default:
            break;
    }
}

static void mapAlignItems(uint8_t v, DSLNode& node) {
    switch (v) {
        case CSS_ALIGN_ITEMS_CENTER:     node.style.setAlignItems(YGAlignCenter); break;
        case CSS_ALIGN_ITEMS_FLEX_END:   node.style.setAlignItems(YGAlignFlexEnd); break;
        case CSS_ALIGN_ITEMS_STRETCH:    node.style.setAlignItems(YGAlignStretch); break;
        case CSS_ALIGN_ITEMS_BASELINE:   node.style.setAlignItems(YGAlignBaseline); break;
        default:                         node.style.setAlignItems(YGAlignFlexStart); break;
    }
}

static void mapJustify(uint8_t v, DSLNode& node) {
    switch (v) {
        case CSS_JUSTIFY_CONTENT_CENTER:        node.style.setJustifyContent(YGJustifyCenter); break;
        case CSS_JUSTIFY_CONTENT_FLEX_END:    node.style.setJustifyContent(YGJustifyFlexEnd); break;
        case CSS_JUSTIFY_CONTENT_SPACE_BETWEEN: node.style.setJustifyContent(YGJustifySpaceBetween); break;
        case CSS_JUSTIFY_CONTENT_SPACE_AROUND:  node.style.setJustifyContent(YGJustifySpaceAround); break;
        case CSS_JUSTIFY_CONTENT_SPACE_EVENLY:  node.style.setJustifyContent(YGJustifySpaceEvenly); break;
        default:                                node.style.setJustifyContent(YGJustifyFlexStart); break;
    }
}

static void mapFlexDir(uint8_t v, DSLNode& node) {
    switch (v) {
        case CSS_FLEX_DIRECTION_ROW:            node.style.setFlexDirection(YGFlexDirectionRow); break;
        case CSS_FLEX_DIRECTION_ROW_REVERSE:    node.style.setFlexDirection(YGFlexDirectionRowReverse); break;
        case CSS_FLEX_DIRECTION_COLUMN:         node.style.setFlexDirection(YGFlexDirectionColumn); break;
        case CSS_FLEX_DIRECTION_COLUMN_REVERSE: node.style.setFlexDirection(YGFlexDirectionColumnReverse); break;
        default: break;
    }
}

static void mapFlexWrap(uint8_t v, DSLNode& node) {
    switch (v) {
        case CSS_FLEX_WRAP_WRAP:         node.style.setFlexWrap(YGWrapWrap); break;
        case CSS_FLEX_WRAP_WRAP_REVERSE: node.style.setFlexWrap(YGWrapWrapReverse); break;
        default:                         node.style.setFlexWrap(YGWrapNoWrap); break;
    }
}

static void mapTextAlign(uint8_t v, DSLNode& node) {
    switch (v) {
        case CSS_TEXT_ALIGN_CENTER:
        case CSS_TEXT_ALIGN_LIBCSS_CENTER:
            node.textAlign = TextAlign::Center;
            break;
        case CSS_TEXT_ALIGN_RIGHT:
        case CSS_TEXT_ALIGN_LIBCSS_RIGHT:
            node.textAlign = TextAlign::Right;
            break;
        default:
            node.textAlign = TextAlign::Left;
            break;
    }
}

static void mapCursor(uint8_t c, DSLNode& node) {
    switch (c) {
        case CSS_CURSOR_POINTER: node.style.cursorType = Cursor::Hand; break;
        case CSS_CURSOR_TEXT:    node.style.cursorType = Cursor::IBeam; break;
        case CSS_CURSOR_E_RESIZE:
        case CSS_CURSOR_W_RESIZE:
            node.style.cursorType = Cursor::SizeWE;
            break;
        case CSS_CURSOR_N_RESIZE:
        case CSS_CURSOR_S_RESIZE:
            node.style.cursorType = Cursor::SizeNS;
            break;
        default: node.style.cursorType = Cursor::Arrow; break;
    }
}

static bool fontWeightBold(uint8_t w) {
    return w == CSS_FONT_WEIGHT_BOLD || w == CSS_FONT_WEIGHT_BOLDER ||
           w == CSS_FONT_WEIGHT_600 || w == CSS_FONT_WEIGHT_700 ||
           w == CSS_FONT_WEIGHT_800 || w == CSS_FONT_WEIGHT_900;
}

static void applyOneBorder(const css_computed_style* s, const css_unit_ctx* unitCtx, DSLNode& node,
                           uint8_t (*widthFn)(const css_computed_style*, css_fixed*, css_unit*),
                           uint8_t (*styleFn)(const css_computed_style*),
                           uint8_t (*colorFn)(const css_computed_style*, css_color*),
                           float* outW, AUKColor* outC) {
    css_fixed bw = 0;
    css_unit bu = CSS_UNIT_PX;
    uint8_t wst = widthFn(s, &bw, &bu);
    uint8_t st = styleFn(s);
    css_color bc = 0;
    uint8_t cst = colorFn(s, &bc);
    if (wst == CSS_BORDER_WIDTH_WIDTH && st != CSS_BORDER_STYLE_NONE && st != CSS_BORDER_STYLE_HIDDEN &&
        (cst == CSS_BORDER_COLOR_COLOR || cst == CSS_BORDER_COLOR_CURRENT_COLOR)) {
        *outW = lenToPx(s, unitCtx, bw, bu);
        *outC = AUKColor(static_cast<SkColor>(bc));
    } else {
        *outW = 0;
        *outC = AUKColor::transparent();
    }
}

static void applyComputed(const css_computed_style* s, DSLNode& node, DslInherited& inh,
                          const css_unit_ctx* unitCtx) {
    /* display */
    uint8_t disp = css_computed_display(s, false);
    mapDisplay(disp, node);

    /* flex */
    mapFlexDir(css_computed_flex_direction(s), node);
    mapAlignItems(css_computed_align_items(s), node);
    mapJustify(css_computed_justify_content(s), node);
    mapFlexWrap(css_computed_flex_wrap(s), node);

    css_fixed fg = 0, fs = 0, fbLen = 0;
    css_unit fbUnit = CSS_UNIT_PX;
    if (css_computed_flex_grow(s, &fg) == CSS_FLEX_GROW_SET)
        node.style.setFlexGrow(fixToFloat(fg));
    if (css_computed_flex_shrink(s, &fs) == CSS_FLEX_SHRINK_SET)
        node.style.setFlexShrink(fixToFloat(fs));
    uint8_t fbStat = css_computed_flex_basis(s, &fbLen, &fbUnit);
    if (fbStat == CSS_FLEX_BASIS_SET)
        node.style.setFlexBasis(lenToPx(s, unitCtx, fbLen, fbUnit));
    else if (fbStat == CSS_FLEX_BASIS_AUTO)
        node.style.setFlexBasis(-1.0f);

    /* sizes */
    css_fixed len = 0;
    css_unit u = CSS_UNIT_PX;
    uint8_t wt = css_computed_width(s, &len, &u);
    if (wt == CSS_WIDTH_SET)
        node.style.setWidth(lenToPx(s, unitCtx, len, u));
    else if (wt == CSS_WIDTH_AUTO)
        node.style.setWidthAuto();

    uint8_t ht = css_computed_height(s, &len, &u);
    if (ht == CSS_HEIGHT_SET)
        node.style.setHeight(lenToPx(s, unitCtx, len, u));
    else if (ht == CSS_HEIGHT_AUTO)
        node.style.setHeightAuto();

    if (css_computed_min_width(s, &len, &u) == CSS_MIN_WIDTH_SET)
        node.style.setMinWidth(lenToPx(s, unitCtx, len, u));
    if (css_computed_min_height(s, &len, &u) == CSS_MIN_HEIGHT_SET)
        node.style.setMinHeight(lenToPx(s, unitCtx, len, u));

    /* padding */
    if (css_computed_padding_top(s, &len, &u) == CSS_PADDING_SET)
        node.style.paddingTop = lenToPx(s, unitCtx, len, u);
    if (css_computed_padding_right(s, &len, &u) == CSS_PADDING_SET)
        node.style.paddingRight = lenToPx(s, unitCtx, len, u);
    if (css_computed_padding_bottom(s, &len, &u) == CSS_PADDING_SET)
        node.style.paddingBottom = lenToPx(s, unitCtx, len, u);
    if (css_computed_padding_left(s, &len, &u) == CSS_PADDING_SET)
        node.style.paddingLeft = lenToPx(s, unitCtx, len, u);

    /* margin */
    if (css_computed_margin_top(s, &len, &u) == CSS_MARGIN_SET)
        node.style.marginTop = lenToPx(s, unitCtx, len, u);
    if (css_computed_margin_right(s, &len, &u) == CSS_MARGIN_SET)
        node.style.marginRight = lenToPx(s, unitCtx, len, u);
    if (css_computed_margin_bottom(s, &len, &u) == CSS_MARGIN_SET)
        node.style.marginBottom = lenToPx(s, unitCtx, len, u);
    if (css_computed_margin_left(s, &len, &u) == CSS_MARGIN_SET)
        node.style.marginLeft = lenToPx(s, unitCtx, len, u);

    /* colours */
    css_color bg = 0;
    if (css_computed_background_color(s, &bg) == CSS_BACKGROUND_COLOR_COLOR)
        node.style.backgroundColor = AUKColor(static_cast<SkColor>(bg));

    css_color col = 0;
    if (css_computed_color(s, &col) == CSS_COLOR_COLOR) {
        node.textColor = AUKColor(static_cast<SkColor>(col));
        inh.color = node.textColor;
        inh.hasColor = true;
    }

    /* overflow */
    uint8_t ox = css_computed_overflow_x(s);
    uint8_t oy = css_computed_overflow_y(s);
    node.style.overflowHidden =
        (ox == CSS_OVERFLOW_HIDDEN || ox == CSS_OVERFLOW_SCROLL || ox == CSS_OVERFLOW_AUTO) ||
        (oy == CSS_OVERFLOW_HIDDEN || oy == CSS_OVERFLOW_SCROLL || oy == CSS_OVERFLOW_AUTO);

    /* typography — `names` is an lwc_string*[]; generic keywords in status. */
    lwc_string** families = nullptr;
    uint8_t ffType = css_computed_font_family(s, &families);
    bool gotNamed = false;
    if (families) {
        for (size_t i = 0; families[i]; ++i) {
            std::string fam = lwcStr(families[i]);
            auto comma = fam.find(',');
            if (comma != std::string::npos) fam = fam.substr(0, comma);
            while (!fam.empty() && fam.front() == ' ') fam.erase(fam.begin());
            while (!fam.empty() && fam.back() == ' ') fam.pop_back();
            if (!fam.empty()) {
                node.fontFamily = fam;
                inh.fontFamily = fam;
                gotNamed = true;
                break;
            }
        }
    }
    if (!gotNamed) {
        if (ffType == CSS_FONT_FAMILY_MONOSPACE) {
            node.fontFamily = FontManager::MONOSPACE_FONT;
            inh.fontFamily = node.fontFamily;
        } else if (ffType == CSS_FONT_FAMILY_SERIF) {
            node.fontFamily = "Times New Roman";
            inh.fontFamily = node.fontFamily;
        } else if (ffType == CSS_FONT_FAMILY_SANS_SERIF || ffType == CSS_FONT_FAMILY_FANTASY ||
                   ffType == CSS_FONT_FAMILY_CURSIVE) {
            node.fontFamily = FontManager::DEFAULT_FONT;
            inh.fontFamily = node.fontFamily;
        }
    }

    if (css_computed_font_size(s, &len, &u) == CSS_FONT_SIZE_DIMENSION) {
        float px = lenToPx(s, unitCtx, len, u);
        node.fontSize = px;
        inh.fontSize = px;
    }

    uint8_t fw = css_computed_font_weight(s);
    node.fontBold = fontWeightBold(fw);
    inh.bold = node.fontBold;

    mapTextAlign(css_computed_text_align(s), node);

    /* borders */
    applyOneBorder(s, unitCtx, node, css_computed_border_top_width, css_computed_border_top_style,
                   css_computed_border_top_color, &node.borderTopWidth, &node.borderTopColor);
    applyOneBorder(s, unitCtx, node, css_computed_border_right_width, css_computed_border_right_style,
                   css_computed_border_right_color, &node.borderRightWidth, &node.borderRightColor);
    applyOneBorder(s, unitCtx, node, css_computed_border_bottom_width, css_computed_border_bottom_style,
                   css_computed_border_bottom_color, &node.borderBottomWidth, &node.borderBottomColor);
    applyOneBorder(s, unitCtx, node, css_computed_border_left_width, css_computed_border_left_style,
                   css_computed_border_left_color, &node.borderLeftWidth, &node.borderLeftColor);

    lwc_string** cursorUrls = nullptr;
    mapCursor(css_computed_cursor(s, &cursorUrls), node);
}

} // anonymous

static std::mutex gLibCssMutex;
static css_select_ctx* gSelectCtx = nullptr;

static css_select_ctx* selectCtx() {
    std::lock_guard<std::mutex> lock(gLibCssMutex);
    if (!gSelectCtx) {
        if (css_select_ctx_create(&gSelectCtx) != CSS_OK)
            return nullptr;
    }
    return gSelectCtx;
}

bool applyLibCssDeclarationBlock(const std::string& cssText, const char* elementTag,
                                 DSLNode& node, DslInherited& inh) {
    if (!cssText.empty() && cssText.find_first_not_of(" \t\r\n") == std::string::npos)
        return true;

    css_select_ctx* ctx = selectCtx();
    if (!ctx) return false;

    const std::string tag = (elementTag && elementTag[0]) ? elementTag : "div";
    lwc_string* tagStr = nullptr;
    lwc_intern_string(tag.c_str(), tag.size(), &tagStr);

    css_stylesheet_params params{};
    params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
    params.level = CSS_LEVEL_3;
    params.charset = "UTF-8";
    params.url = "inline://dsl";
    params.title = "dsl-inline";
    params.allow_quirks = false;
    params.inline_style = true;
    params.resolve = resolve_url;
    params.resolve_pw = nullptr;
    params.import = nullptr;
    params.import_pw = nullptr;
    params.color = nullptr;
    params.color_pw = nullptr;
    params.font = nullptr;
    params.font_pw = nullptr;

    css_stylesheet* sheet = nullptr;
    if (css_stylesheet_create(&params, &sheet) != CSS_OK) {
        lwc_string_unref(tagStr);
        return false;
    }

    std::string data = cssText;
    if (!data.empty() && data.back() != ';')
        data.push_back(';');

    css_error err = css_stylesheet_append_data(sheet, reinterpret_cast<const uint8_t*>(data.data()),
                                               data.size());
    if (err != CSS_OK && err != CSS_NEEDDATA) {
        css_stylesheet_destroy(sheet);
        lwc_string_unref(tagStr);
        return false;
    }
    if (css_stylesheet_data_done(sheet) != CSS_OK) {
        css_stylesheet_destroy(sheet);
        lwc_string_unref(tagStr);
        return false;
    }

    static css_unit_ctx unitCtx = makeUnitCtx();
    static css_media media = makeMedia();

    css_select_results* results = nullptr;
    err = css_select_style(ctx, tagStr, &unitCtx, &media, sheet, &kSelectHandler, nullptr, &results);
    css_stylesheet_destroy(sheet);
    lwc_string_unref(tagStr);

    if (err != CSS_OK || !results || !results->styles[CSS_PSEUDO_ELEMENT_NONE]) {
        if (results) css_select_results_destroy(results);
        return false;
    }

    css_computed_style* comp = results->styles[CSS_PSEUDO_ELEMENT_NONE];
    applyComputed(comp, node, inh, &unitCtx);

    css_select_results_destroy(results);
    return true;
}

} // namespace AureliaUI
