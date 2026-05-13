#include <SPHXGraphicInterface.hpp>
#include <SPHXGraphicComponents.hpp>
#include <SphereVUE.hpp>
#include <core/ResourceManager.hpp>
#include <VueUIResources.hpp>

#include <windows.h>
#include <string>

using namespace SphereUI;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Application::getInstance().init();

    InitVueUIResources();

    const std::string bundleSrc =
        ResourceManager::getInstance().getResourceString("res://bundle.js");

    if (bundleSrc.empty()) {
        Win32Window window("VueUI Demo - Error", 800, 400);
        window.setDarkMode(true);
        window.enableMica(true);
        window.setRoot(MakeJSErrorScreen(
            "Vue",
            "Resource not found: res://bundle.js",
            "The embedded JavaScript bundle could not be loaded from the resource pack.",
            "Run `bun run bundle:demo` in modules/vueui/ to build the JS bundle from example/App.vue, "
            "then rebuild the CMake target so gen_resources.ts embeds it."));
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
        window.setRoot(MakeJSErrorScreen(
            "Vue",
            "JavaScript evaluation failed",
            err,
            "The bundle loaded, but the JS engine returned an evaluation error before a root node was produced.",
            engine.debugLog()));
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
