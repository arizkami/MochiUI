#include <SPHXGraphicInterface.hpp>
#include <SPHXGraphicComponents.hpp>
#include <SphereVUE.hpp>
#include <core/ResourceManager.hpp>
#include <VueUIResources.hpp>

#include <windows.h>
#include <string>

using namespace SphereUI;

static FlexNode::Ptr MakeErrorRoot(const std::string& heading, const std::string& detail) {
    auto root = FlexNode::Column();
    root->style.backgroundColor = SPHXColor::Hex("#0d0d11");
    root->style.setWidthFull();
    root->style.setHeightFull();
    root->style.setPadding(32);
    root->style.setGap(10);

    auto h = std::make_shared<TextNode>(heading);
    h->color = SPHXColor::Hex("#ff6b6b");
    h->fontSize = 18.0f;
    h->fontBold = true;
    root->addChild(h);

    if (!detail.empty()) {
        auto d = std::make_shared<TextNode>(detail);
        d->color = SPHXColor::Hex("#c0c0d0");
        d->fontSize = 13.0f;
        root->addChild(d);
    }

    return root;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Application::getInstance().init();

    InitVueUIResources();

    const std::string bundleSrc =
        ResourceManager::getInstance().getResourceString("res://bundle.js");

    if (bundleSrc.empty()) {
        Win32Window window("VueUI Demo - Error", 800, 400);
        window.setDarkMode(true);
        window.enableMica(true);
        window.setRoot(MakeErrorRoot(
            "Resource not found: res://bundle.js",
            "Run `bun run bundle:demo` in modules/vueui/ to build the JS bundle from example/App.vue,\n"
            "then rebuild the CMake target so gen_resources.py embeds it."));
        window.run();
        return 1;
    }

    JavaScriptEngine engine;
    engine.init();

    auto root = RenderVueApp(engine, bundleSrc, "res://bundle.js");

    if (!root) {
        const std::string err = engine.lastError();
        Win32Window window("VueUI Demo - JS Error", 900, 500);
        window.setDarkMode(true);
        window.enableMica(true);
        window.setRoot(MakeErrorRoot("JavaScript evaluation failed", err));
        window.run();
        return 1;
    }

    Win32Window window("SphereKit - Vue UI Demo", 1280, 800);
    window.setDarkMode(true);
    window.enableMica(true);
    window.center();
    window.setRoot(root);
    window.run();
    return 0;
}
