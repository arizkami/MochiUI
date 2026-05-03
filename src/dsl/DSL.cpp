#include <AUKDSL.hpp>
#include <dsl/DslInherited.hpp>
#include <dsl/LibCssStyle.hpp>

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace AureliaUI {

// ══════════════════════════════════════════════════════════════════════════════
//  DSLNode
// ══════════════════════════════════════════════════════════════════════════════

DSLNode::DSLNode() {
    // Default to row + auto-sizing — closer to CSS than the FlexNode default of
    // column + Hug. The parser overrides whichever bits the YAML specifies.
    style.flexDirection = FlexDirection::Row;
    YGNodeStyleSetFlexDirection(getYGNode(), YGFlexDirectionRow);
}

void DSLNode::setLeafTextNode(bool leaf) {
    YGNodeSetMeasureFunc(getYGNode(), leaf ? &FlexNode::MeasureCallback : nullptr);
}

Size DSLNode::measure(Size /*available*/) {
    if (text.empty()) return { 0, 0 };

    SkRect ink = SkRect::MakeEmpty();
    if (fontBold) {
        SkFont font = FontManager::getInstance().createFont(fontFamily, fontSize, SkFontStyle::Bold());
        font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8, &ink, nullptr);
    } else {
        FontManager::getInstance().measureText(text, fontSize, &ink, fontFamily);
    }

    float w = std::max(0.0f, ink.width());
    float h = std::max(0.0f, ink.height());
    if (!(h > 0.5f)) {
        SkFontMetrics m{};
        if (fontBold) {
            SkFont font = FontManager::getInstance().createFont(fontFamily, fontSize, SkFontStyle::Bold());
            font.getMetrics(&m);
        } else {
            FontManager::getInstance().getFontMetrics(fontSize, &m, fontFamily);
        }
        h = std::abs(m.fAscent) + std::abs(m.fDescent);
    }
    if (!(w > 0.5f)) {
        w = FontManager::getInstance().measureText(text, fontSize, nullptr, fontFamily);
    }

    if (letterSpacing > 0 && text.size() > 1)
        w += letterSpacing * float(std::max<size_t>(1, text.size()) - 1);

    w += style.paddingLeft + style.paddingRight;
    h += style.paddingTop + style.paddingBottom;
    return { std::ceil(w), std::ceil(h) };
}

void DSLNode::drawSelf(SkCanvas* canvas) {
    // 1. Background + hover overlay (delegate to FlexNode).
    FlexNode::drawSelf(canvas);

    // 2. Borders. Drawn as inset rectangles so they don't bleed past
    //    `borderRadius`-clipped corners.
    auto edge = [&](float w, AUKColor c, SkRect r) {
        if (w <= 0 || SkColorGetA(c) == 0) return;
        SkPaint p; p.setAntiAlias(true); p.setColor(c);
        canvas->drawRect(r, p);
    };
    edge(borderTopWidth,    borderTopColor,
         SkRect::MakeLTRB(frame.left(),  frame.top(),
                          frame.right(), frame.top() + borderTopWidth));
    edge(borderBottomWidth, borderBottomColor,
         SkRect::MakeLTRB(frame.left(),  frame.bottom() - borderBottomWidth,
                          frame.right(), frame.bottom()));
    edge(borderLeftWidth,   borderLeftColor,
         SkRect::MakeLTRB(frame.left(),                       frame.top(),
                          frame.left() + borderLeftWidth,     frame.bottom()));
    edge(borderRightWidth,  borderRightColor,
         SkRect::MakeLTRB(frame.right() - borderRightWidth,   frame.top(),
                          frame.right(),                      frame.bottom()));
}

void DSLNode::draw(SkCanvas* canvas) {
    drawSelf(canvas);

    if (!text.empty()) {
        canvas->save();
        if (style.overflowHidden)
            canvas->clipRect(frame);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(textColor);

        SkRect ink = SkRect::MakeEmpty();
        if (fontBold) {
            SkFont font = FontManager::getInstance().createFont(fontFamily, fontSize, SkFontStyle::Bold());
            font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8, &ink, nullptr);
        } else {
            FontManager::getInstance().measureText(text, fontSize, &ink, fontFamily);
        }

        float trackExtra = 0;
        if (letterSpacing > 0 && text.size() > 1)
            trackExtra = letterSpacing * float(std::max<size_t>(1, text.size()) - 1);

        float tw = std::max(0.0f, ink.width()) + trackExtra;
        if (!(tw > 0.5f))
            tw = FontManager::getInstance().measureText(text, fontSize, nullptr, fontFamily) + trackExtra;

        float padL = getLayoutPadding(YGEdgeLeft);
        float padR = getLayoutPadding(YGEdgeRight);
        float x = frame.left() + padL;
        if (textAlign == TextAlign::Center)
            x = frame.left() + (frame.width() - tw) * 0.5f;
        else if (textAlign == TextAlign::Right)
            x = frame.right() - padR - tw;

        const float midY = frame.centerY();
        float baseline = midY;
        if (ink.height() > 0.5f)
            baseline = midY - (ink.top() + ink.bottom()) * 0.5f;
        else {
            SkFontMetrics m{};
            if (fontBold) {
                SkFont font = FontManager::getInstance().createFont(fontFamily, fontSize, SkFontStyle::Bold());
                font.getMetrics(&m);
            } else {
                FontManager::getInstance().getFontMetrics(fontSize, &m, fontFamily);
            }
            baseline = midY - (m.fAscent + m.fDescent) * 0.5f;
        }

        if (letterSpacing <= 0) {
            if (fontBold) {
                SkFont font = FontManager::getInstance().createFont(fontFamily, fontSize, SkFontStyle::Bold());
                canvas->drawSimpleText(text.c_str(), text.size(), SkTextEncoding::kUTF8,
                                       std::round(x), std::round(baseline), font, paint);
            } else {
                FontManager::getInstance().drawText(canvas, text, std::round(x), std::round(baseline),
                                                    fontSize, paint, fontFamily);
            }
        } else {
            bool ascii = true;
            for (unsigned char c : text) {
                if (c >= 128) {
                    ascii = false;
                    break;
                }
            }
            if (!ascii) {
                if (fontBold) {
                    SkFont font = FontManager::getInstance().createFont(fontFamily, fontSize, SkFontStyle::Bold());
                    canvas->drawSimpleText(text.c_str(), text.size(), SkTextEncoding::kUTF8,
                                           std::round(x), std::round(baseline), font, paint);
                } else {
                    FontManager::getInstance().drawText(canvas, text, std::round(x), std::round(baseline),
                                                        fontSize, paint, fontFamily);
                }
            } else {
                float cx = x;
                for (size_t i = 0; i < text.size(); ++i) {
                    std::string one(1, text[i]);
                    float cw = FontManager::getInstance().measureText(one, fontSize, nullptr, fontFamily);
                    if (fontBold) {
                        SkFont font = FontManager::getInstance().createFont(fontFamily, fontSize, SkFontStyle::Bold());
                        canvas->drawSimpleText(one.c_str(), one.size(), SkTextEncoding::kUTF8,
                                               std::round(cx), std::round(baseline), font, paint);
                    } else {
                        FontManager::getInstance().drawText(canvas, one, std::round(cx), std::round(baseline),
                                                            fontSize, paint, fontFamily);
                    }
                    cx += cw + letterSpacing;
                }
            }
        }

        canvas->restore();
    }

    drawChildren(canvas);
}

// ══════════════════════════════════════════════════════════════════════════════
//  Parser helpers
// ══════════════════════════════════════════════════════════════════════════════

namespace {

// ── String utilities ─────────────────────────────────────────────────────────

std::string trim(std::string s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::vector<std::string> splitWhitespace(const std::string& s) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string tok;
    while (ss >> tok) out.push_back(tok);
    return out;
}

// ── Length parsing ───────────────────────────────────────────────────────────
// Strips "px"/"%"/etc. and parses the leading numeric value. "auto"/"none"/""
// resolve to NaN so callers can detect "unset".

bool parseLength(const std::string& raw, float& out) {
    std::string s = trim(raw);
    if (s.empty()) return false;
    std::string lo = toLower(s);
    if (lo == "auto" || lo == "none" || lo == "inherit") return false;

    // Strip a trailing CSS unit suffix (px, pt, em, rem, %).
    size_t end = 0;
    while (end < s.size() &&
           (std::isdigit((unsigned char)s[end]) || s[end] == '.' ||
            s[end] == '-' || s[end] == '+'))
        ++end;
    if (end == 0) return false;
    try {
        out = std::stof(s.substr(0, end));
    } catch (...) { return false; }
    return true;
}

float parseLengthOr(const std::string& raw, float fallback) {
    float v;
    return parseLength(raw, v) ? v : fallback;
}

// ── Color parsing ────────────────────────────────────────────────────────────
// Accepts "#RGB", "#RRGGBB", "#RRGGBBAA", "rgb(...)", "rgba(...)", "transparent".

bool parseColor(const std::string& raw, AUKColor& out) {
    std::string s = trim(raw);
    if (s.empty()) return false;
    std::string lo = toLower(s);

    if (lo == "transparent" || lo == "none") {
        out = AUKColor::transparent();
        return true;
    }
    if (lo == "white") { out = AUKColor::white(); return true; }
    if (lo == "black") { out = AUKColor::black(); return true; }
    if (lo == "red")   { out = AUKColor::red();   return true; }
    if (lo == "green") { out = AUKColor::green(); return true; }
    if (lo == "blue")  { out = AUKColor::blue();  return true; }

    if (s[0] == '#') { out = AUKColor::Hex(s); return true; }

    if (lo.rfind("rgb", 0) == 0) {
        size_t lp = s.find('('); size_t rp = s.find(')');
        if (lp == std::string::npos || rp == std::string::npos) return false;
        std::string inner = s.substr(lp + 1, rp - lp - 1);
        std::replace(inner.begin(), inner.end(), ',', ' ');
        auto toks = splitWhitespace(inner);
        if (toks.size() < 3) return false;
        int r = std::stoi(toks[0]);
        int g = std::stoi(toks[1]);
        int b = std::stoi(toks[2]);
        int a = 255;
        if (toks.size() >= 4) {
            float af = std::stof(toks[3]);
            a = (af > 1.0f) ? (int)af : (int)std::round(af * 255.0f);
        }
        out = AUKColor::RGB((uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a);
        return true;
    }
    return false;
}

// ── 1/2/3/4 value shorthand → top/right/bottom/left ─────────────────────────

struct Edges { float t = 0, r = 0, b = 0, l = 0; bool any = false; };

Edges parseEdges(const std::string& raw) {
    Edges e;
    auto toks = splitWhitespace(trim(raw));
    if (toks.empty()) return e;

    auto px = [](const std::string& s) { float v = 0; parseLength(s, v); return v; };
    switch (toks.size()) {
        case 1: e.t = e.r = e.b = e.l = px(toks[0]); break;
        case 2: e.t = e.b = px(toks[0]); e.r = e.l = px(toks[1]); break;
        case 3: e.t = px(toks[0]); e.r = e.l = px(toks[1]); e.b = px(toks[2]); break;
        default:
            e.t = px(toks[0]); e.r = px(toks[1]);
            e.b = px(toks[2]); e.l = px(toks[3]); break;
    }
    e.any = true;
    return e;
}

// ── Border shorthand: "<width> [style] <color>" ─────────────────────────────

struct BorderSpec { float width = 0; AUKColor color = AUKColor::transparent(); bool any = false; };

BorderSpec parseBorder(const std::string& raw) {
    BorderSpec b;
    auto toks = splitWhitespace(trim(raw));
    for (auto& t : toks) {
        float w; AUKColor c;
        // "solid" / "dashed" / "dotted" / "none" — ignored, we only render solid.
        std::string lo = toLower(t);
        if (lo == "solid" || lo == "dashed" || lo == "dotted" ||
            lo == "double" || lo == "none" || lo == "groove" || lo == "ridge")
            continue;
        if (parseLength(t, w))      { b.width = w; b.any = true; }
        else if (parseColor(t, c))  { b.color = c; b.any = true; }
    }
    return b;
}

// ── Style enums ──────────────────────────────────────────────────────────────

YGAlign parseAlign(const std::string& raw) {
    std::string v = toLower(trim(raw));
    if (v == "center")     return YGAlignCenter;
    if (v == "flex-end" || v == "end") return YGAlignFlexEnd;
    if (v == "stretch")    return YGAlignStretch;
    if (v == "baseline")   return YGAlignBaseline;
    return YGAlignFlexStart;
}

YGJustify parseJustify(const std::string& raw) {
    std::string v = toLower(trim(raw));
    if (v == "center")        return YGJustifyCenter;
    if (v == "flex-end" || v == "end") return YGJustifyFlexEnd;
    if (v == "space-between") return YGJustifySpaceBetween;
    if (v == "space-around")  return YGJustifySpaceAround;
    if (v == "space-evenly")  return YGJustifySpaceEvenly;
    return YGJustifyFlexStart;
}

YGFlexDirection parseDirection(const std::string& raw, YGFlexDirection fallback) {
    std::string v = toLower(trim(raw));
    if (v == "row")            return YGFlexDirectionRow;
    if (v == "row-reverse")    return YGFlexDirectionRowReverse;
    if (v == "column")         return YGFlexDirectionColumn;
    if (v == "column-reverse") return YGFlexDirectionColumnReverse;
    return fallback;
}

YGWrap parseWrap(const std::string& raw) {
    std::string v = toLower(trim(raw));
    if (v == "wrap")         return YGWrapWrap;
    if (v == "wrap-reverse") return YGWrapWrapReverse;
    return YGWrapNoWrap;
}

bool parseFontWeight(const std::string& raw, bool& boldOut) {
    std::string v = toLower(trim(raw));
    if (v.empty()) return false;
    if (v == "bold" || v == "bolder") { boldOut = true;  return true; }
    if (v == "normal" || v == "lighter") { boldOut = false; return true; }
    int n = 0;
    try { n = std::stoi(v); } catch (...) { return false; }
    boldOut = (n >= 600);
    return true;
}

// ── YAML helpers ─────────────────────────────────────────────────────────────

std::string asString(const YAML::Node& n) {
    if (!n) return {};
    try { return n.as<std::string>(); } catch (...) { return {}; }
}

// Look up `key` in `node` and `fallback` (in that order) so the parser is
// permissive about whether `children`/`text` live inside `props` or beside it.
YAML::Node lookup(const YAML::Node& primary, const YAML::Node& fallback,
                  const std::string& key) {
    if (primary && primary.IsMap()) {
        auto v = primary[key];
        if (v) return v;
    }
    if (fallback && fallback.IsMap()) {
        auto v = fallback[key];
        if (v) return v;
    }
    return YAML::Node();
}

void applyStyle(DSLNode& node, const YAML::Node& style, DslInherited& inh) {
    if (!style || !style.IsMap()) return;

    auto get = [&](const char* key) -> std::string {
        auto v = style[key];
        return v ? asString(v) : std::string{};
    };

    // ── Sizing ───────────────────────────────────────────────────────────────
    if (auto v = style["width"]) {
        std::string s = asString(v);
        std::string lo = toLower(trim(s));
        if (lo == "auto") node.style.setWidthAuto();
        else if (!s.empty() && s.back() == '%') {
            float p = 0; parseLength(s, p);
            node.style.setWidthPercent(p);
        } else {
            float w = 0;
            if (parseLength(s, w)) node.style.setWidth(w);
        }
    }
    if (auto v = style["height"]) {
        std::string s = asString(v);
        std::string lo = toLower(trim(s));
        if (lo == "auto") node.style.setHeightAuto();
        else if (!s.empty() && s.back() == '%') {
            float p = 0; parseLength(s, p);
            node.style.setHeightPercent(p);
        } else {
            float h = 0;
            if (parseLength(s, h)) node.style.setHeight(h);
        }
    }
    if (auto v = style["minWidth"]) { float w; if (parseLength(asString(v), w)) node.style.setMinWidth(w); }
    if (auto v = style["minHeight"]){ float h; if (parseLength(asString(v), h)) node.style.setMinHeight(h); }

    // ── Flex container behaviour ─────────────────────────────────────────────
    YGFlexDirection dir = (node.style.flexDirection == FlexDirection::Row)
                            ? YGFlexDirectionRow : YGFlexDirectionColumn;
    bool isFlex = false;
    if (auto v = style["display"]) {
        std::string d = toLower(trim(asString(v)));
        if (d == "flex" || d == "inline-flex") {
            isFlex = true;
            // CSS default for flex containers is row.
            dir = YGFlexDirectionRow;
        }
    }
    if (auto v = style["flexDirection"]) {
        dir = parseDirection(asString(v), dir);
        isFlex = true;
    }
    if (isFlex) {
        node.style.setFlexDirection(dir);
    }

    if (auto v = style["alignItems"])     node.style.setAlignItems(parseAlign(asString(v)));
    if (auto v = style["justifyContent"]) node.style.setJustifyContent(parseJustify(asString(v)));
    if (auto v = style["flexWrap"])       node.style.setFlexWrap(parseWrap(asString(v)));

    if (auto v = style["flex"])       { float f; if (parseLength(asString(v), f)) node.style.setFlex(f); }
    if (auto v = style["flexGrow"])   { float f; if (parseLength(asString(v), f)) node.style.setFlexGrow(f); }
    if (auto v = style["flexShrink"]) { float f; if (parseLength(asString(v), f)) node.style.setFlexShrink(f); }
    if (auto v = style["flexBasis"])  { float f; if (parseLength(asString(v), f)) node.style.setFlexBasis(f); }
    if (auto v = style["gap"])        { float g; if (parseLength(asString(v), g)) node.style.setGap(g); }

    // ── Box model ────────────────────────────────────────────────────────────
    if (auto v = style["padding"]) {
        auto e = parseEdges(asString(v));
        if (e.any) node.style.setPadding(e.l, e.t, e.r, e.b);
    }
    if (auto v = style["paddingTop"])    { float f; if (parseLength(asString(v), f)) node.style.paddingTop = f; }
    if (auto v = style["paddingRight"])  { float f; if (parseLength(asString(v), f)) node.style.paddingRight = f; }
    if (auto v = style["paddingBottom"]) { float f; if (parseLength(asString(v), f)) node.style.paddingBottom = f; }
    if (auto v = style["paddingLeft"])   { float f; if (parseLength(asString(v), f)) node.style.paddingLeft = f; }

    if (auto v = style["margin"]) {
        auto e = parseEdges(asString(v));
        if (e.any) {
            node.style.marginTop    = e.t;
            node.style.marginRight  = e.r;
            node.style.marginBottom = e.b;
            node.style.marginLeft   = e.l;
        }
    }

    // ── Visual ───────────────────────────────────────────────────────────────
    if (auto v = style["backgroundColor"]) {
        AUKColor c; if (parseColor(asString(v), c)) node.style.backgroundColor = c;
    }
    if (auto v = style["color"]) {
        AUKColor c; if (parseColor(asString(v), c)) {
            node.textColor = c;
            inh.color    = c;
            inh.hasColor = true;
        }
    }
    if (auto v = style["borderRadius"]) {
        float r; if (parseLength(asString(v), r)) node.style.borderRadius = r;
    }
    if (auto v = style["overflow"]) {
        node.style.overflowHidden = (toLower(trim(asString(v))) == "hidden");
    }

    // ── Borders (shorthand + per-edge) ───────────────────────────────────────
    auto applyBorder = [&](const BorderSpec& b, bool top, bool right, bool bot, bool left) {
        if (!b.any) return;
        if (top)   { node.borderTopWidth    = b.width; node.borderTopColor    = b.color; }
        if (right) { node.borderRightWidth  = b.width; node.borderRightColor  = b.color; }
        if (bot)   { node.borderBottomWidth = b.width; node.borderBottomColor = b.color; }
        if (left)  { node.borderLeftWidth   = b.width; node.borderLeftColor   = b.color; }
    };
    if (auto v = style["border"])       applyBorder(parseBorder(asString(v)), true, true, true, true);
    if (auto v = style["borderTop"])    applyBorder(parseBorder(asString(v)), true, false, false, false);
    if (auto v = style["borderRight"])  applyBorder(parseBorder(asString(v)), false, true, false, false);
    if (auto v = style["borderBottom"]) applyBorder(parseBorder(asString(v)), false, false, true, false);
    if (auto v = style["borderLeft"])   applyBorder(parseBorder(asString(v)), false, false, false, true);
    if (auto v = style["borderWidth"]) {
        float w; if (parseLength(asString(v), w)) {
            node.borderTopWidth = node.borderRightWidth =
            node.borderBottomWidth = node.borderLeftWidth = w;
        }
    }
    if (auto v = style["borderColor"]) {
        AUKColor c; if (parseColor(asString(v), c)) {
            node.borderTopColor = node.borderRightColor =
            node.borderBottomColor = node.borderLeftColor = c;
        }
    }

    // ── Typography ───────────────────────────────────────────────────────────
    if (auto v = style["fontFamily"]) {
        std::string f = asString(v);
        if (!f.empty()) {
            // CSS-style "Inter, sans-serif" — take the first family.
            auto comma = f.find(',');
            if (comma != std::string::npos) f = f.substr(0, comma);
            node.fontFamily = trim(f);
            inh.fontFamily  = node.fontFamily;
        }
    }
    if (auto v = style["fontSize"]) {
        float f; if (parseLength(asString(v), f)) {
            node.fontSize = f;
            inh.fontSize  = f;
        }
    }
    if (auto v = style["fontWeight"]) {
        bool b = false;
        if (parseFontWeight(asString(v), b)) {
            node.fontBold = b;
            inh.bold      = b;
        }
    }
    if (auto v = style["textAlign"]) {
        std::string t = toLower(trim(asString(v)));
        if (t == "center")      node.textAlign = TextAlign::Center;
        else if (t == "right")  node.textAlign = TextAlign::Right;
        else                    node.textAlign = TextAlign::Left;
    }

    // ── Cursor (best-effort mapping) ─────────────────────────────────────────
    if (auto v = style["cursor"]) {
        std::string c = toLower(trim(asString(v)));
        if      (c == "pointer") node.style.cursorType = Cursor::Hand;
        else if (c == "text")    node.style.cursorType = Cursor::IBeam;
        else if (c == "ns-resize") node.style.cursorType = Cursor::SizeNS;
        else if (c == "ew-resize") node.style.cursorType = Cursor::SizeWE;
        else                       node.style.cursorType = Cursor::Arrow;
    }

    if (auto v = style["letterSpacing"]) {
        std::string s = trim(asString(v));
        float em = 0;
        if (s.size() > 2 && toLower(s.substr(s.size() - 2)) == "em") {
            if (parseLength(s.substr(0, s.size() - 2), em))
                node.letterSpacing = em * node.fontSize;
        } else {
            float px = 0;
            if (parseLength(s, px))
                node.letterSpacing = px;
        }
    }
}

// Forward decl: build a node tree recursively.
DSLNode::Ptr buildNode(const YAML::Node& spec, DslInherited inherited);

DSLNode::Ptr buildNode(const YAML::Node& spec, DslInherited inherited) {
    if (!spec || !spec.IsMap()) return nullptr;

    auto node = std::make_shared<DSLNode>();

    // Seed the node from the inherited cascade.
    if (!inherited.fontFamily.empty()) node->fontFamily = inherited.fontFamily;
    node->fontSize = inherited.fontSize;
    node->fontBold = inherited.bold;
    if (inherited.hasColor) node->textColor = inherited.color;

    // ── type: div | span ────────────────────────────────────────────────────
    std::string type = "div";
    if (auto t = spec["type"]) type = toLower(trim(asString(t)));

    // ── props.* (style + text + maybe children) ─────────────────────────────
    YAML::Node props = spec["props"];
    YAML::Node style = props ? props["style"] : YAML::Node();

    DslInherited childInh = inherited;
    if (style && style.IsScalar()) {
        const std::string cssBlock = style.as<std::string>();
        applyLibCssDeclarationBlock(cssBlock, type.c_str(), *node, childInh);
    } else {
        applyStyle(*node, style, childInh);
    }

    // Inline text — accept either props.text or, leniently, props.style.text.
    std::string text;
    if (props) {
        if (auto t = props["text"]) text = asString(t);
    }
    if (text.empty() && style && style.IsMap()) {
        if (auto t = style["text"]) text = asString(t);
    }
    node->text = text;

    // ── children: peer of `props` OR inside `props` ─────────────────────────
    YAML::Node children = lookup(spec, props, "children");

    // Honour CSS "block" defaults for `<div>` when no explicit display/direction
    // was given: divs stack vertically, spans flow inline (row).
    if (style && style.IsMap()) {
        bool hasDisplay   = (bool)style["display"];
        bool hasDirection = (bool)style["flexDirection"];
        if (!hasDisplay && !hasDirection) {
            if (type == "div") {
                node->style.setFlexDirection(YGFlexDirectionColumn);
            }
        }
    } else if (type == "div") {
        node->style.setFlexDirection(YGFlexDirectionColumn);
    }

    // Build children before deciding leaf-ness, because a node with text AND
    // children should keep its measure callback off (Yoga only honours measure
    // on true leaves).
    bool hasChildren = false;
    if (children && children.IsSequence()) {
        for (const auto& childSpec : children) {
            auto child = buildNode(childSpec, childInh);
            if (child) {
                node->addChild(child);
                hasChildren = true;
            }
        }
    }

    node->setLeafTextNode(!text.empty() && !hasChildren);
    return node;
}

} // namespace

// ══════════════════════════════════════════════════════════════════════════════
//  Public API
// ══════════════════════════════════════════════════════════════════════════════

namespace DSL {

FlexNode::Ptr loadFromString(const std::string& yaml) {
    YAML::Node doc;
    try {
        doc = YAML::Load(yaml);
    } catch (const YAML::Exception&) {
        return nullptr;
    }
    if (!doc) return nullptr;

    // Document root is either the node spec, or a single-key wrapper around it
    // (e.g. `navbar: { type: div, ... }`).
    YAML::Node spec = doc;
    if (doc.IsMap() && !doc["type"]) {
        // Pick the first value if exactly one key, otherwise treat the whole
        // map as the spec (will fail gracefully if neither matches).
        if (doc.size() == 1) {
            spec = doc.begin()->second;
        }
    }

    return buildNode(spec, DslInherited{});
}

FlexNode::Ptr loadFromFile(const std::string& path) {
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in) return nullptr;
    std::ostringstream ss; ss << in.rdbuf();
    return loadFromString(ss.str());
}

} // namespace DSL
} // namespace AureliaUI
