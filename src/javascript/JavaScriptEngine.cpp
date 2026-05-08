#include <AUKJavaScriptEngine.hpp>

#include <libplatform/libplatform.h>
#include <v8.h>

#include <chrono>
#include <cstring>
#include <memory>
#include <mutex>
#include <unordered_map>
#if defined(_WIN32)
#include <windows.h>   // OutputDebugStringA
#endif

namespace AureliaUI {

namespace {

// ── Registry ─────────────────────────────────────────────────────────────────

struct UiRegistry {
    uint32_t nextId = 1;
    std::unordered_map<uint32_t, FlexNode::Ptr> nodes;
    std::unordered_map<uint32_t, uint32_t>      contentProxy; // scrollId → inner content id
    FlexNode::Ptr                               pendingRoot;

    uint32_t store(FlexNode::Ptr node) {
        const uint32_t id = nextId++;
        nodes[id] = std::move(node);
        return id;
    }
    FlexNode::Ptr take(uint32_t id) {
        auto it = nodes.find(id);
        if (it == nodes.end()) return nullptr;
        FlexNode::Ptr n = it->second;
        nodes.erase(it);
        return n;
    }
    FlexNode::Ptr get(uint32_t id) const {
        auto it = nodes.find(id);
        return it == nodes.end() ? nullptr : it->second;
    }
    uint32_t resolveProxy(uint32_t id) const {
        auto it = contentProxy.find(id);
        return it != contentProxy.end() ? it->second : id;
    }
};

static std::unordered_map<v8::Isolate*, UiRegistry*> gRegByIsolate;

static UiRegistry* regFor(v8::Isolate* iso) {
    auto it = gRegByIsolate.find(iso);
    return it == gRegByIsolate.end() ? nullptr : it->second;
}

// ── V8 helpers ────────────────────────────────────────────────────────────────

static std::string toUtf8(v8::Isolate* isolate, v8::Local<v8::String> s) {
    v8::String::Utf8Value u(isolate, s);
    return u.length() ? std::string(*u, u.length()) : std::string{};
}

static uint32_t toUint32(v8::Isolate* isolate, v8::Local<v8::Value> v, bool& ok) {
    ok = false;
    if (v.IsEmpty() || !v->IsNumber()) return 0;
    double d = v->NumberValue(isolate->GetCurrentContext()).FromMaybe(0);
    if (d < 0 || d > static_cast<double>(0xFFFFFFFFu)) return 0;
    ok = true;
    return static_cast<uint32_t>(d);
}

static void Throw(v8::Isolate* isolate, const char* msg) {
    isolate->ThrowException(
        v8::String::NewFromUtf8(isolate, msg, v8::NewStringType::kNormal).ToLocalChecked());
}

// ── API: createNode ───────────────────────────────────────────────────────────

static void ApiCreateNode(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    v8::HandleScope hs(isolate);
    UiRegistry* reg = regFor(isolate);
    if (!reg) { Throw(isolate, "AureliaUI: no registry"); return; }
    if (args.Length() < 1 || !args[0]->IsString()) {
        Throw(isolate, "createNode(type: string)");
        return;
    }
    const std::string type = toUtf8(isolate, args[0].As<v8::String>());

    // ScrollView / FlatList: transparent content proxy so appendChild targets the inner node
    if (type == "ScrollView" || type == "FlatList" || type == "scroll") {
        auto scroll  = std::make_shared<ScrollAreaNode>();
        auto content = FlexNode::Column();
        content->style.setWidthFull();
        scroll->setContent(content);
        const uint32_t scrollId  = reg->store(scroll);
        const uint32_t contentId = reg->store(content);
        reg->contentProxy[scrollId] = contentId;
        args.GetReturnValue().Set(static_cast<double>(scrollId));
        return;
    }
    if (type == "HScrollView") {
        auto scroll  = std::make_shared<ScrollAreaNode>();
        scroll->setHorizontal(true);
        auto content = FlexNode::Row();
        content->style.setHeightFull();
        scroll->setContent(content);
        const uint32_t scrollId  = reg->store(scroll);
        const uint32_t contentId = reg->store(content);
        reg->contentProxy[scrollId] = contentId;
        args.GetReturnValue().Set(static_cast<double>(scrollId));
        return;
    }

    FlexNode::Ptr node;
    if      (type == "View" || type == "SafeAreaView" ||
             type == "column" || type == "div")       { node = FlexNode::Column(); }
    else if (type == "row")                            { node = FlexNode::Row(); }
    else if (type == "Text" || type == "text")        { node = std::make_shared<TextNode>(); }
    else if (type == "Button")                         { node = std::make_shared<ButtonNode>(); }
    else if (type == "TouchableOpacity" ||
             type == "Pressable") {
        node = FlexNode::Column();
        node->enableHover = true;
    }
    else if (type == "TextInput")                      { node = std::make_shared<TextInput>(); }
    else if (type == "Switch")                         { node = std::make_shared<SwitchNode>(); }
    else if (type == "Slider")                         { node = std::make_shared<SliderNode>(); }
    else if (type == "Image")                          { node = FlexNode::Create(); }
    else                                               { node = FlexNode::Create(); }

    args.GetReturnValue().Set(static_cast<double>(reg->store(std::move(node))));
}

// ── API: appendChild / removeChild ────────────────────────────────────────────

static void ApiAppendChild(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    UiRegistry* reg = regFor(isolate);
    if (!reg) { Throw(isolate, "AureliaUI: no registry"); return; }
    bool ok = false;
    uint32_t pid = toUint32(isolate, args[0], ok);
    if (!ok) { Throw(isolate, "appendChild(parentId, childId)"); return; }
    const uint32_t cid = toUint32(isolate, args[1], ok);
    if (!ok) { Throw(isolate, "appendChild(parentId, childId)"); return; }

    pid = reg->resolveProxy(pid); // transparently reroute scroll containers
    FlexNode::Ptr parent = reg->get(pid);
    FlexNode::Ptr child  = reg->get(cid);
    if (!parent || !child) { Throw(isolate, "appendChild: invalid handle"); return; }
    parent->addChild(child);
}

static void ApiRemoveChild(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    UiRegistry* reg = regFor(isolate);
    if (!reg) { Throw(isolate, "AureliaUI: no registry"); return; }
    bool ok = false;
    uint32_t pid = toUint32(isolate, args[0], ok);
    if (!ok) { Throw(isolate, "removeChild(parentId, childId)"); return; }
    const uint32_t cid = toUint32(isolate, args[1], ok);
    if (!ok) { Throw(isolate, "removeChild(parentId, childId)"); return; }

    pid = reg->resolveProxy(pid);
    FlexNode::Ptr parent = reg->get(pid);
    FlexNode::Ptr child  = reg->get(cid);
    if (!parent || !child) { Throw(isolate, "removeChild: invalid handle"); return; }
    parent->removeChild(child);
}

// ── API: setText ──────────────────────────────────────────────────────────────

static void ApiSetText(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    UiRegistry* reg = regFor(isolate);
    if (!reg) { Throw(isolate, "AureliaUI: no registry"); return; }
    bool ok = false;
    const uint32_t id = toUint32(isolate, args[0], ok);
    if (!ok || args.Length() < 2 || !args[1]->IsString()) {
        Throw(isolate, "setText(id, text)");
        return;
    }
    FlexNode::Ptr node = reg->get(id);
    if (!node) { Throw(isolate, "setText: invalid handle"); return; }

    const std::string text = toUtf8(isolate, args[1].As<v8::String>());
    if (auto* tn = dynamic_cast<TextNode*>(node.get())) {
        tn->text = text;
    } else {
        node->removeAllChildren();
        node->addChild(std::make_shared<TextNode>(text));
    }
}

// ── Style key dispatcher ──────────────────────────────────────────────────────

static bool applyStyleKey(FlexNode& node, const std::string& key, const std::string& val) {
    float f = 0;
    auto pf = [&](float& out) { return sscanf(val.c_str(), "%f", &out) == 1; };

    if (key == "width") {
        if (val == "100%" || val == "full") node.style.setWidthFull();
        else if (!val.empty() && val.back() == '%' && pf(f)) node.style.setWidthPercent(f);
        else if (val == "auto") node.style.setWidthAuto();
        else if (pf(f)) node.style.setWidth(f);
        return true;
    }
    if (key == "height") {
        if (val == "100%" || val == "full") node.style.setHeightFull();
        else if (!val.empty() && val.back() == '%' && pf(f)) node.style.setHeightPercent(f);
        else if (val == "auto") node.style.setHeightAuto();
        else if (pf(f)) node.style.setHeight(f);
        return true;
    }
    if (key == "minWidth")  { if (pf(f)) node.style.setMinWidth(f);  return true; }
    if (key == "minHeight") { if (pf(f)) node.style.setMinHeight(f); return true; }

    if (key == "padding")          { if (pf(f)) node.style.setPadding(f);        return true; }
    if (key == "paddingTop")       { if (pf(f)) node.style.paddingTop    = f;    return true; }
    if (key == "paddingBottom")    { if (pf(f)) node.style.paddingBottom = f;    return true; }
    if (key == "paddingLeft")      { if (pf(f)) node.style.paddingLeft   = f;    return true; }
    if (key == "paddingRight")     { if (pf(f)) node.style.paddingRight  = f;    return true; }
    if (key == "paddingHorizontal"){ if (pf(f)) { node.style.paddingLeft = node.style.paddingRight = f; } return true; }
    if (key == "paddingVertical")  { if (pf(f)) { node.style.paddingTop  = node.style.paddingBottom = f; } return true; }

    if (key == "margin")           { if (pf(f)) node.style.setMargin(f);         return true; }
    if (key == "marginTop")        { if (pf(f)) node.style.marginTop    = f;     return true; }
    if (key == "marginBottom")     { if (pf(f)) node.style.marginBottom = f;     return true; }
    if (key == "marginLeft")       { if (pf(f)) node.style.marginLeft   = f;     return true; }
    if (key == "marginRight")      { if (pf(f)) node.style.marginRight  = f;     return true; }
    if (key == "marginHorizontal") { if (pf(f)) { node.style.marginLeft = node.style.marginRight = f; } return true; }
    if (key == "marginVertical")   { if (pf(f)) { node.style.marginTop  = node.style.marginBottom = f; } return true; }

    if (key == "flex")       { if (pf(f)) node.style.setFlex(f);       return true; }
    if (key == "flexGrow")   { if (pf(f)) node.style.setFlexGrow(f);   return true; }
    if (key == "flexShrink") { if (pf(f)) node.style.setFlexShrink(f); return true; }
    if (key == "flexBasis")  { if (pf(f)) node.style.setFlexBasis(f);  return true; }
    if (key == "flexWrap") {
        node.style.setFlexWrap(val == "wrap" ? YGWrapWrap : YGWrapNoWrap);
        return true;
    }
    if (key == "gap") { if (pf(f)) node.style.setGap(f); return true; }

    if (key == "borderRadius") { if (pf(f)) node.style.borderRadius = f; return true; }
    if (key == "overflow")     { node.style.overflowHidden = (val == "hidden"); return true; }

    if (key == "backgroundColor") {
        node.style.backgroundColor = AUKColor::Hex(val.c_str());
        return true;
    }
    if (key == "display") {
        if (val == "flex") node.style.setFlexDirection(YGFlexDirectionRow);
        return true;
    }
    if (key == "flexDirection") {
        node.style.setFlexDirection(val == "row" ? YGFlexDirectionRow : YGFlexDirectionColumn);
        return true;
    }
    if (key == "alignItems") {
        if      (val == "center")               node.style.setAlignItems(YGAlignCenter);
        else if (val == "flex-end" || val == "end") node.style.setAlignItems(YGAlignFlexEnd);
        else if (val == "stretch")              node.style.setAlignItems(YGAlignStretch);
        else                                    node.style.setAlignItems(YGAlignFlexStart);
        return true;
    }
    if (key == "justifyContent") {
        if      (val == "center")        node.style.setJustifyContent(YGJustifyCenter);
        else if (val == "flex-end" || val == "end") node.style.setJustifyContent(YGJustifyFlexEnd);
        else if (val == "space-between") node.style.setJustifyContent(YGJustifySpaceBetween);
        else if (val == "space-around")  node.style.setJustifyContent(YGJustifySpaceAround);
        else if (val == "space-evenly")  node.style.setJustifyContent(YGJustifySpaceEvenly);
        else                             node.style.setJustifyContent(YGJustifyFlexStart);
        return true;
    }
    if (key == "position") {
        node.style.setPositionType(val == "absolute" ? YGPositionTypeAbsolute : YGPositionTypeStatic);
        return true;
    }
    if (key == "top")    { if (pf(f)) node.style.setPosition(YGEdgeTop,    f); return true; }
    if (key == "left")   { if (pf(f)) node.style.setPosition(YGEdgeLeft,   f); return true; }
    if (key == "right")  { if (pf(f)) node.style.setPosition(YGEdgeRight,  f); return true; }
    if (key == "bottom") { if (pf(f)) node.style.setPosition(YGEdgeBottom, f); return true; }

    // Text-specific style properties — dispatch if node is a TextNode
    if (key == "color") {
        if (auto* t = dynamic_cast<TextNode*>(&node)) t->color = AUKColor::Hex(val.c_str());
        return true;
    }
    if (key == "fontSize") {
        if (pf(f)) {
            if (auto* t = dynamic_cast<TextNode*>(&node))   t->fontSize  = f;
            if (auto* b = dynamic_cast<ButtonNode*>(&node)) b->fontSize  = f;
        }
        return true;
    }
    if (key == "fontWeight") {
        bool bold = (val == "bold" || val == "700" || val == "800" || val == "900");
        if (auto* t = dynamic_cast<TextNode*>(&node))   t->fontBold   = bold;
        if (auto* b = dynamic_cast<ButtonNode*>(&node)) b->labelBold  = bold;
        return true;
    }
    if (key == "textAlign") {
        if (auto* t = dynamic_cast<TextNode*>(&node)) {
            if      (val == "center") t->textAlign = TextAlign::Center;
            else if (val == "right")  t->textAlign = TextAlign::Right;
            else                      t->textAlign = TextAlign::Left;
        }
        return true;
    }
    return false;
}

static void ApiSetStyle(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    UiRegistry* reg = regFor(isolate);
    if (!reg) { Throw(isolate, "AureliaUI: no registry"); return; }
    bool ok = false;
    const uint32_t id = toUint32(isolate, args[0], ok);
    if (!ok || args.Length() < 3 || !args[1]->IsString() || !args[2]->IsString()) {
        Throw(isolate, "setStyle(id, key, value)");
        return;
    }
    FlexNode::Ptr node = reg->get(id);
    if (!node) { Throw(isolate, "setStyle: invalid handle"); return; }
    applyStyleKey(*node,
                  toUtf8(isolate, args[1].As<v8::String>()),
                  toUtf8(isolate, args[2].As<v8::String>()));
}

// ── API: setProp ──────────────────────────────────────────────────────────────
// Component-specific data properties (non-style, non-event).

static void ApiSetProp(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    UiRegistry* reg = regFor(isolate);
    if (!reg) { Throw(isolate, "AureliaUI: no registry"); return; }
    bool ok = false;
    const uint32_t id = toUint32(isolate, args[0], ok);
    if (!ok || args.Length() < 3 || !args[1]->IsString() || !args[2]->IsString()) {
        Throw(isolate, "setProp(id, key, value)");
        return;
    }
    FlexNode::Ptr node = reg->get(id);
    if (!node) { Throw(isolate, "setProp: invalid handle"); return; }

    const std::string key = toUtf8(isolate, args[1].As<v8::String>());
    const std::string val = toUtf8(isolate, args[2].As<v8::String>());
    float f = 0;

    if (auto* btn = dynamic_cast<ButtonNode*>(node.get())) {
        if (key == "title" || key == "label" || key == "text") btn->label = val;
        else if (key == "fontSize")   { if (sscanf(val.c_str(), "%f", &f) == 1) btn->fontSize  = f; }
        else if (key == "fontWeight") btn->labelBold = (val == "bold" || val == "700");
    } else if (auto* tn = dynamic_cast<TextNode*>(node.get())) {
        if      (key == "text")       tn->text = val;
        else if (key == "fontSize")   { if (sscanf(val.c_str(), "%f", &f) == 1) tn->fontSize = f; }
        else if (key == "fontWeight") tn->fontBold = (val == "bold" || val == "700");
        else if (key == "color")      tn->color = AUKColor::Hex(val.c_str());
        else if (key == "textAlign") {
            if      (val == "center") tn->textAlign = TextAlign::Center;
            else if (val == "right")  tn->textAlign = TextAlign::Right;
            else                      tn->textAlign = TextAlign::Left;
        }
    } else if (auto* sl = dynamic_cast<SliderNode*>(node.get())) {
        if      (key == "value")                              { if (sscanf(val.c_str(), "%f", &f) == 1) sl->value    = f; }
        else if (key == "minimumValue" || key == "minValue") { if (sscanf(val.c_str(), "%f", &f) == 1) sl->minValue = f; }
        else if (key == "maximumValue" || key == "maxValue") { if (sscanf(val.c_str(), "%f", &f) == 1) sl->maxValue = f; }
        else if (key == "vertical") sl->vertical = (val == "true");
    } else if (auto* sw = dynamic_cast<SwitchNode*>(node.get())) {
        if      (key == "value" || key == "isOn" || key == "checked") sw->isOn = (val == "true" || val == "1");
        else if (key == "label") sw->label = val;
    } else if (auto* ti = dynamic_cast<TextInput*>(node.get())) {
        if      (key == "value" || key == "text") ti->text        = val;
        else if (key == "placeholder")            ti->placeholder = val;
    } else if (auto* sc = dynamic_cast<ScrollAreaNode*>(node.get())) {
        if (key == "horizontal") sc->setHorizontal(val == "true");
    }
}

// ── API: setCallback ──────────────────────────────────────────────────────────
// Wire a JS function to a native component event callback.
// Works within a single-threaded event loop where native events fire while V8 is idle.

static void ApiSetCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    UiRegistry* reg = regFor(isolate);
    if (!reg) { Throw(isolate, "AureliaUI: no registry"); return; }
    bool ok = false;
    const uint32_t id = toUint32(isolate, args[0], ok);
    if (!ok || args.Length() < 3 || !args[1]->IsString() || !args[2]->IsFunction()) {
        Throw(isolate, "setCallback(id, event, fn)");
        return;
    }
    FlexNode::Ptr node = reg->get(id);
    if (!node) { Throw(isolate, "setCallback: invalid handle"); return; }

    const std::string event = toUtf8(isolate, args[1].As<v8::String>());
    v8::Local<v8::Function> fn  = args[2].As<v8::Function>();
    v8::Local<v8::Context>  ctx = isolate->GetCurrentContext();

    // v8::Global<> is move-only, so lambdas that capture it by move are
    // non-copyable and can't be stored in std::function.  Wrap in shared_ptr
    // so the lambda becomes copyable.
    using SGFn  = std::shared_ptr<v8::Global<v8::Function>>;
    using SGCtx = std::shared_ptr<v8::Global<v8::Context>>;
    auto cbG  = std::make_shared<v8::Global<v8::Function>>(isolate, fn);
    auto ctxG = std::make_shared<v8::Global<v8::Context>>(isolate, ctx);

    if (event == "onPress" || event == "onClick") {
        node->onClick = [isolate, ctxG, cbG]() {
            v8::Isolate::Scope iso(isolate);
            v8::HandleScope    hs(isolate);
            v8::Local<v8::Context>  lctx = ctxG->Get(isolate);
            v8::Context::Scope cs(lctx);
            v8::Local<v8::Function> lfn  = cbG->Get(isolate);
            (void)lfn->Call(lctx, v8::Undefined(isolate), 0, nullptr);
        };
    } else if (event == "onValueChange" || event == "onChanged") {
        if (auto* sl = dynamic_cast<SliderNode*>(node.get())) {
            sl->onValueChange = [isolate, ctxG, cbG](float val) {
                v8::Isolate::Scope iso(isolate);
                v8::HandleScope    hs(isolate);
                v8::Local<v8::Context>  lctx = ctxG->Get(isolate);
                v8::Context::Scope cs(lctx);
                v8::Local<v8::Function> lfn  = cbG->Get(isolate);
                v8::Local<v8::Value>    arg  = v8::Number::New(isolate, val);
                (void)lfn->Call(lctx, v8::Undefined(isolate), 1, &arg);
            };
        } else if (auto* sw = dynamic_cast<SwitchNode*>(node.get())) {
            sw->onChanged = [isolate, ctxG, cbG](bool val) {
                v8::Isolate::Scope iso(isolate);
                v8::HandleScope    hs(isolate);
                v8::Local<v8::Context>  lctx = ctxG->Get(isolate);
                v8::Context::Scope cs(lctx);
                v8::Local<v8::Function> lfn  = cbG->Get(isolate);
                v8::Local<v8::Value>    arg  = v8::Boolean::New(isolate, val);
                (void)lfn->Call(lctx, v8::Undefined(isolate), 1, &arg);
            };
        }
    } else if (event == "onChangeText" || event == "onChange") {
        if (auto* ti = dynamic_cast<TextInput*>(node.get())) {
            ti->onChanged = [isolate, ctxG, cbG](const std::string& val) {
                v8::Isolate::Scope iso(isolate);
                v8::HandleScope    hs(isolate);
                v8::Local<v8::Context>  lctx = ctxG->Get(isolate);
                v8::Context::Scope cs(lctx);
                v8::Local<v8::Function> lfn  = cbG->Get(isolate);
                auto ms = v8::String::NewFromUtf8(isolate, val.c_str(),
                    v8::NewStringType::kNormal, static_cast<int>(val.size()));
                if (!ms.IsEmpty()) {
                    v8::Local<v8::Value> arg = ms.ToLocalChecked();
                    (void)lfn->Call(lctx, v8::Undefined(isolate), 1, &arg);
                }
            };
        }
    } else if (event == "onSubmitEditing" || event == "onEnter") {
        if (auto* ti = dynamic_cast<TextInput*>(node.get())) {
            ti->onEnter = [isolate, ctxG, cbG]() {
                v8::Isolate::Scope iso(isolate);
                v8::HandleScope    hs(isolate);
                v8::Local<v8::Context>  lctx = ctxG->Get(isolate);
                v8::Context::Scope cs(lctx);
                v8::Local<v8::Function> lfn  = cbG->Get(isolate);
                (void)lfn->Call(lctx, v8::Undefined(isolate), 0, nullptr);
            };
        }
    }
}

// ── API: setRoot ──────────────────────────────────────────────────────────────

static void ApiSetRoot(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    UiRegistry* reg = regFor(isolate);
    if (!reg) { Throw(isolate, "AureliaUI: no registry"); return; }
    bool ok = false;
    const uint32_t id = toUint32(isolate, args[0], ok);
    if (!ok) { Throw(isolate, "setRoot(id)"); return; }
    FlexNode::Ptr node = reg->get(id);
    if (!node) { Throw(isolate, "setRoot: invalid handle"); return; }
    reg->pendingRoot = node;
}

// ── Global polyfill callbacks ─────────────────────────────────────────────────

static void ApiNoop(const v8::FunctionCallbackInfo<v8::Value>&) {}

static void ApiReturnZero(const v8::FunctionCallbackInfo<v8::Value>& args) {
    args.GetReturnValue().Set(0);
}

static void ApiConsoleLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* iso = args.GetIsolate();
    v8::HandleScope hs(iso);
    std::string msg;
    for (int i = 0; i < args.Length(); ++i) {
        if (i > 0) msg += ' ';
        v8::String::Utf8Value s(iso, args[i]);
        if (s.length() > 0) msg.append(*s, s.length());
    }
    msg += '\n';
#if defined(_WIN32)
    OutputDebugStringA(msg.c_str());
#else
    fputs(msg.c_str(), stderr);
#endif
}

static void ApiPerformanceNow(const v8::FunctionCallbackInfo<v8::Value>& args) {
    using namespace std::chrono;
    double ms = static_cast<double>(
        duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count()) / 1000.0;
    args.GetReturnValue().Set(ms);
}

// ── Build API object ──────────────────────────────────────────────────────────

static v8::Local<v8::Object> makeAureliaUIApi(v8::Isolate* isolate) {
    v8::EscapableHandleScope hs(isolate);
    v8::Local<v8::Context>   ctx = isolate->GetCurrentContext();
    v8::Local<v8::Object>    api = v8::Object::New(isolate);

    auto bind = [&](const char* name, v8::FunctionCallback cb) {
        v8::Local<v8::Function> fn =
            v8::Function::New(ctx, cb, {}, 0, v8::ConstructorBehavior::kThrow)
                .ToLocalChecked();
        (void)api->Set(ctx,
            v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kNormal).ToLocalChecked(),
            fn);
    };

    bind("createNode",   ApiCreateNode);
    bind("appendChild",  ApiAppendChild);
    bind("removeChild",  ApiRemoveChild);
    bind("setText",      ApiSetText);
    bind("setStyle",     ApiSetStyle);
    bind("setProp",      ApiSetProp);
    bind("setCallback",  ApiSetCallback);
    bind("setRoot",      ApiSetRoot);

    return hs.Escape(api);
}

} // namespace

// ── JavaScriptEngine implementation ───────────────────────────────────────────

struct JavaScriptEngine::Impl {
    std::unique_ptr<v8::Platform>     platform;
    v8::Isolate*                      isolate   = nullptr;
    v8::Global<v8::Context>           context;
    v8::ArrayBuffer::Allocator*       allocator = nullptr;
    bool                              installedUi = false;
    std::unique_ptr<UiRegistry>       registry;

    ~Impl() { shutdown(); }

    void shutdown() {
        if (isolate) {
            if (registry) {
                gRegByIsolate.erase(isolate);
                registry.reset();
            }
            isolate->Dispose();
            isolate = nullptr;
        }
        if (allocator) { delete allocator; allocator = nullptr; }
        if (platform) {
            v8::V8::Dispose();
            v8::V8::DisposePlatform();
            platform.reset();
        }
    }
};

JavaScriptEngine::JavaScriptEngine() : impl_(std::make_unique<Impl>()) {}
JavaScriptEngine::~JavaScriptEngine() { shutdown(); }

bool JavaScriptEngine::init() {
    if (impl_->isolate) return true;

    static std::once_flag v8_once;
    std::call_once(v8_once, [] {
        const char* flags = "--no-lazy";
        v8::V8::SetFlagsFromString(flags, static_cast<int>(strlen(flags)));
    });

    impl_->platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(impl_->platform.get());
    v8::V8::Initialize();

    impl_->allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = impl_->allocator;
    impl_->isolate = v8::Isolate::New(params);

    impl_->registry = std::make_unique<UiRegistry>();
    gRegByIsolate[impl_->isolate] = impl_->registry.get();

    {
        v8::Isolate::Scope is(impl_->isolate);
        v8::HandleScope    hs(impl_->isolate);
        v8::Local<v8::Context> ctx = v8::Context::New(impl_->isolate);
        impl_->context.Reset(impl_->isolate, ctx);
    }
    return true;
}

void JavaScriptEngine::shutdown() {
    if (!impl_) return;
    impl_->shutdown();
    lastError_.clear();
}

bool JavaScriptEngine::eval(std::string_view source, std::string_view filename) {
    lastError_.clear();
    if (!impl_->isolate) {
        if (!init()) { lastError_ = "JavaScriptEngine::init failed"; return false; }
    }

    v8::Isolate* const     isolate = impl_->isolate;
    v8::Isolate::Scope     is(isolate);
    v8::HandleScope        hs(isolate);
    v8::Local<v8::Context> ctx = impl_->context.Get(isolate);
    v8::Context::Scope     cs(ctx);
    v8::TryCatch           tc(isolate);

    v8::Local<v8::String> src =
        v8::String::NewFromUtf8(isolate, source.data(), v8::NewStringType::kNormal,
                                static_cast<int>(source.size())).ToLocalChecked();
    v8::Local<v8::String> name =
        v8::String::NewFromUtf8(isolate, filename.data(), v8::NewStringType::kNormal,
                                static_cast<int>(filename.size())).ToLocalChecked();

    v8::ScriptOrigin origin(name);
    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(ctx, src, &origin).ToLocal(&script)) {
        v8::String::Utf8Value err(isolate, tc.Exception());
        lastError_.assign(*err, err.length());
        return false;
    }
    v8::Local<v8::Value> result;
    if (!script->Run(ctx).ToLocal(&result)) {
        v8::String::Utf8Value err(isolate, tc.Exception());
        lastError_.assign(*err, err.length());
        return false;
    }
    return true;
}

void JavaScriptEngine::installAureliaUIGlobal() {
    if (!impl_->isolate) init();
    v8::Isolate* const     isolate = impl_->isolate;
    v8::Isolate::Scope     is(isolate);
    v8::HandleScope        hs(isolate);
    v8::Local<v8::Context> ctx = impl_->context.Get(isolate);
    v8::Context::Scope     cs(ctx);
    v8::Local<v8::Object>  global = ctx->Global();

    auto str = [&](const char* k) {
        return v8::String::NewFromUtf8(isolate, k, v8::NewStringType::kNormal).ToLocalChecked();
    };
    auto fn = [&](v8::FunctionCallback cb) {
        return v8::Function::New(ctx, cb, {}, 0, v8::ConstructorBehavior::kThrow).ToLocalChecked();
    };
    auto setG = [&](const char* name, v8::Local<v8::Value> val) {
        (void)global->Set(ctx, str(name), val);
    };

    // ── AureliaUI native bridge ───────────────────────────────────────────────
    setG("AureliaUI", makeAureliaUIApi(isolate));

    // ── console polyfill (output → debugger / stderr) ─────────────────────────
    {
        v8::Local<v8::Object> console = v8::Object::New(isolate);
        for (const char* m : { "log", "error", "warn", "info", "debug" })
            (void)console->Set(ctx, str(m), fn(ApiConsoleLog));
        setG("console", console);
    }

    // ── Timer stubs (React scheduler fallback path) ───────────────────────────
    for (const char* name : { "setTimeout", "setInterval", "requestAnimationFrame" })
        setG(name, fn(ApiReturnZero));
    for (const char* name : { "clearTimeout", "clearInterval", "cancelAnimationFrame",
                               "setImmediate", "clearImmediate" })
        setG(name, fn(ApiNoop));

    // ── performance.now polyfill ──────────────────────────────────────────────
    {
        v8::Local<v8::Object> perf = v8::Object::New(isolate);
        (void)perf->Set(ctx, str("now"), fn(ApiPerformanceNow));
        setG("performance", perf);
    }

    impl_->installedUi = true;
}

FlexNode::Ptr JavaScriptEngine::takePendingRoot() {
    if (!impl_->registry) return nullptr;
    FlexNode::Ptr r = std::move(impl_->registry->pendingRoot);
    impl_->registry->pendingRoot.reset();
    return r;
}

} // namespace AureliaUI
