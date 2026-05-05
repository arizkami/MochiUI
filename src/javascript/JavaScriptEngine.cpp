#include <AUKJavaScriptEngine.hpp>

#include <libplatform/libplatform.h>
#include <v8.h>

#include <cstring>
#include <memory>
#include <mutex>
#include <sstream>
#include <unordered_map>

namespace AureliaUI {

namespace {

struct UiRegistry {
    uint32_t nextId = 1;
    std::unordered_map<uint32_t, FlexNode::Ptr> nodes;
    FlexNode::Ptr pendingRoot;

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
};

static std::unordered_map<v8::Isolate*, UiRegistry*> gRegByIsolate;

static UiRegistry* regFor(v8::Isolate* iso) {
    auto it = gRegByIsolate.find(iso);
    return it == gRegByIsolate.end() ? nullptr : it->second;
}

static std::string toUtf8(v8::Isolate* isolate, v8::Local<v8::String> s) {
    v8::String::Utf8Value utf8(isolate, s);
    return utf8.length() ? std::string(*utf8, utf8.length()) : std::string{};
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
    isolate->ThrowException(v8::String::NewFromUtf8(isolate, msg, v8::NewStringType::kNormal).ToLocalChecked());
}

static void ApiCreateNode(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    UiRegistry* reg = regFor(isolate);
    if (!reg) {
        Throw(isolate, "AureliaUI: no registry");
        return;
    }
    if (args.Length() < 1 || !args[0]->IsString()) {
        Throw(isolate, "AureliaUI.createNode(type: string)");
        return;
    }
    const std::string type = toUtf8(isolate, args[0].As<v8::String>());
    FlexNode::Ptr node;
    if (type == "column") {
        node = FlexNode::Column();
    } else if (type == "row") {
        node = FlexNode::Row();
    } else {
        node = FlexNode::Create();
        if (type == "div") {
            node->style.setFlexDirection(YGFlexDirectionColumn);
        }
    }
    const uint32_t id = reg->store(std::move(node));
    args.GetReturnValue().Set(static_cast<double>(id));
}

static void ApiAppendChild(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    UiRegistry* reg = regFor(isolate);
    if (!reg) {
        Throw(isolate, "AureliaUI: no registry");
        return;
    }
    bool ok = false;
    const uint32_t pid = toUint32(isolate, args[0], ok);
    if (!ok) {
        Throw(isolate, "appendChild(parentId, childId)");
        return;
    }
    const uint32_t cid = toUint32(isolate, args[1], ok);
    if (!ok) {
        Throw(isolate, "appendChild(parentId, childId)");
        return;
    }
    FlexNode::Ptr parent = reg->get(pid);
    FlexNode::Ptr child  = reg->get(cid);
    if (!parent || !child) {
        Throw(isolate, "appendChild: invalid handle");
        return;
    }
    parent->addChild(child);
}

static void ApiRemoveChild(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    UiRegistry* reg = regFor(isolate);
    if (!reg) {
        Throw(isolate, "AureliaUI: no registry");
        return;
    }
    bool ok = false;
    const uint32_t pid = toUint32(isolate, args[0], ok);
    if (!ok) {
        Throw(isolate, "removeChild(parentId, childId)");
        return;
    }
    const uint32_t cid = toUint32(isolate, args[1], ok);
    if (!ok) {
        Throw(isolate, "removeChild(parentId, childId)");
        return;
    }
    FlexNode::Ptr parent = reg->get(pid);
    FlexNode::Ptr child  = reg->get(cid);
    if (!parent || !child) {
        Throw(isolate, "removeChild: invalid handle");
        return;
    }
    parent->removeChild(child);
}

static void ApiSetText(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    UiRegistry* reg = regFor(isolate);
    if (!reg) {
        Throw(isolate, "AureliaUI: no registry");
        return;
    }
    bool ok = false;
    const uint32_t id = toUint32(isolate, args[0], ok);
    if (!ok || args.Length() < 2 || !args[1]->IsString()) {
        Throw(isolate, "setText(id, text)");
        return;
    }
    FlexNode::Ptr node = reg->get(id);
    if (!node) {
        Throw(isolate, "setText: invalid handle");
        return;
    }
    node->removeAllChildren();
    auto text = std::make_shared<TextNode>(toUtf8(isolate, args[1].As<v8::String>()));
    node->addChild(text);
}

static bool applyStyleKey(FlexNode& node, const std::string& key, const std::string& val) {
    if (key == "width") {
        float w = 0;
        if (val == "100%" || val == "full") {
            node.style.setWidthFull();
            return true;
        }
        if (sscanf(val.c_str(), "%f", &w) == 1) node.style.setWidth(w);
        return true;
    }
    if (key == "height") {
        float h = 0;
        if (val == "100%" || val == "full") {
            node.style.setHeightFull();
            return true;
        }
        if (sscanf(val.c_str(), "%f", &h) == 1) node.style.setHeight(h);
        return true;
    }
    if (key == "padding") {
        float p = 0;
        if (sscanf(val.c_str(), "%f", &p) == 1) node.style.setPadding(p);
        return true;
    }
    if (key == "backgroundColor") {
        node.style.backgroundColor = AUKColor::Hex(val.c_str());
        return true;
    }
    if (key == "display") {
        if (val == "flex") {
            node.style.setFlexDirection(YGFlexDirectionRow);
        }
        return true;
    }
    if (key == "flexDirection") {
        if (val == "row") node.style.setFlexDirection(YGFlexDirectionRow);
        else if (val == "column") node.style.setFlexDirection(YGFlexDirectionColumn);
        return true;
    }
    if (key == "alignItems") {
        if (val == "center") node.style.setAlignItems(YGAlignCenter);
        else if (val == "flex-end" || val == "end") node.style.setAlignItems(YGAlignFlexEnd);
        else if (val == "stretch") node.style.setAlignItems(YGAlignStretch);
        else node.style.setAlignItems(YGAlignFlexStart);
        return true;
    }
    if (key == "justifyContent") {
        if (val == "center") node.style.setJustifyContent(YGJustifyCenter);
        else if (val == "flex-end" || val == "end") node.style.setJustifyContent(YGJustifyFlexEnd);
        else if (val == "space-between") node.style.setJustifyContent(YGJustifySpaceBetween);
        else node.style.setJustifyContent(YGJustifyFlexStart);
        return true;
    }
    if (key == "gap") {
        float g = 0;
        if (sscanf(val.c_str(), "%f", &g) == 1) node.style.setGap(g);
        return true;
    }
    return false;
}

static void ApiSetStyle(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    UiRegistry* reg = regFor(isolate);
    if (!reg) {
        Throw(isolate, "AureliaUI: no registry");
        return;
    }
    bool ok = false;
    const uint32_t id = toUint32(isolate, args[0], ok);
    if (!ok || args.Length() < 3 || !args[1]->IsString() || !args[2]->IsString()) {
        Throw(isolate, "setStyle(id, key, value)");
        return;
    }
    FlexNode::Ptr node = reg->get(id);
    if (!node) {
        Throw(isolate, "setStyle: invalid handle");
        return;
    }
    const std::string k = toUtf8(isolate, args[1].As<v8::String>());
    const std::string v = toUtf8(isolate, args[2].As<v8::String>());
    applyStyleKey(*node, k, v);
}

static void ApiSetRoot(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* const isolate = args.GetIsolate();
    UiRegistry* reg = regFor(isolate);
    if (!reg) {
        Throw(isolate, "AureliaUI: no registry");
        return;
    }
    bool ok = false;
    const uint32_t id = toUint32(isolate, args[0], ok);
    if (!ok) {
        Throw(isolate, "setRoot(id)");
        return;
    }
    FlexNode::Ptr node = reg->get(id);
    if (!node) {
        Throw(isolate, "setRoot: invalid handle");
        return;
    }
    reg->pendingRoot = node;
}

static v8::Local<v8::Object> makeAureliaUIApi(v8::Isolate* isolate) {
    v8::EscapableHandleScope hs(isolate);
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Object> api = v8::Object::New(isolate);

    auto bind = [&](const char* name, v8::FunctionCallback cb) {
        v8::Local<v8::Function> fn =
            v8::Function::New(ctx, cb, {}, 0, v8::ConstructorBehavior::kThrow).ToLocalChecked();
        (void)api->Set(ctx,
                       v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kNormal).ToLocalChecked(),
                       fn);
    };

    bind("createNode", ApiCreateNode);
    bind("appendChild", ApiAppendChild);
    bind("removeChild", ApiRemoveChild);
    bind("setText", ApiSetText);
    bind("setStyle", ApiSetStyle);
    bind("setRoot", ApiSetRoot);

    return hs.Escape(api);
}

} // namespace

struct JavaScriptEngine::Impl {
    std::unique_ptr<v8::Platform> platform;
    v8::Isolate*                 isolate = nullptr;
    v8::Global<v8::Context>       context;
    v8::ArrayBuffer::Allocator*    allocator = nullptr;
    bool                           installedUi = false;
    std::unique_ptr<UiRegistry>   registry;

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
        if (allocator) {
            delete allocator;
            allocator = nullptr;
        }
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
        v8::Isolate::Scope isolate_scope(impl_->isolate);
        v8::HandleScope handle_scope(impl_->isolate);
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
        if (!init()) {
            lastError_ = "JavaScriptEngine::init failed";
            return false;
        }
    }

    v8::Isolate* const isolate = impl_->isolate;
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = impl_->context.Get(isolate);
    v8::Context::Scope context_scope(ctx);

    v8::TryCatch try_catch(isolate);

    v8::Local<v8::String> src =
        v8::String::NewFromUtf8(isolate, source.data(), v8::NewStringType::kNormal,
                                 static_cast<int>(source.size()))
            .ToLocalChecked();

    v8::Local<v8::String> name =
        v8::String::NewFromUtf8(isolate, filename.data(), v8::NewStringType::kNormal,
                                static_cast<int>(filename.size()))
            .ToLocalChecked();

    // V8 monolith builds often expose the older ScriptOrigin(Value) constructor.
    v8::ScriptOrigin origin(name);

    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(ctx, src, &origin).ToLocal(&script)) {
        v8::String::Utf8Value err(isolate, try_catch.Exception());
        lastError_.assign(*err, err.length());
        return false;
    }

    v8::Local<v8::Value> result;
    if (!script->Run(ctx).ToLocal(&result)) {
        v8::String::Utf8Value err(isolate, try_catch.Exception());
        lastError_.assign(*err, err.length());
        return false;
    }

    return true;
}

void JavaScriptEngine::installAureliaUIGlobal() {
    if (!impl_->isolate) init();
    v8::Isolate* const isolate = impl_->isolate;
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = impl_->context.Get(isolate);
    v8::Context::Scope context_scope(ctx);

    v8::Local<v8::Object> global = ctx->Global();
    v8::Local<v8::String> key =
        v8::String::NewFromUtf8(isolate, "AureliaUI", v8::NewStringType::kNormal).ToLocalChecked();
    v8::Local<v8::Object> api = makeAureliaUIApi(isolate);
    (void)global->Set(ctx, key, api);
    impl_->installedUi = true;
}

FlexNode::Ptr JavaScriptEngine::takePendingRoot() {
    if (!impl_->registry) return nullptr;
    FlexNode::Ptr r = std::move(impl_->registry->pendingRoot);
    impl_->registry->pendingRoot.reset();
    return r;
}

} // namespace AureliaUI
