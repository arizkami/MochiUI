#pragma once

#include <AureliaReact.hpp>
#include <AUKApplication.hpp>
#include <AUKGraphicInterface.hpp>

namespace AureliaUI::MikoUI {

struct AppConfig {
    std::string title = "MikoUI";
    int width = 1180;
    int height = 720;
    bool darkMode = true;
    bool mica = true;
};

// Minimal helper: init app + create window + mount a JS bundle (React reconciler).
inline int Run(const AppConfig& cfg, const std::string& jsBundle, const std::string& filename = "reactui.bundle.js") {
    Application::getInstance().init();

    JavaScriptEngine engine;
    engine.init();

    FlexNode::Ptr root = RenderReactApp(engine, jsBundle, filename);
    if (!root) {
        root = FlexNode::Column();
        root->style.setWidthFull();
        root->style.setHeightFull();
        auto err = std::make_shared<TextNode>("Failed to mount React app: " + engine.lastError());
        err->color = AUKColor::Hex("#ff6b6b");
        err->style.setPadding(16);
        root->addChild(err);
    }

    Win32Window window(cfg.title, cfg.width, cfg.height);
    window.enableMica(cfg.mica);
    window.setDarkMode(cfg.darkMode);
    window.setRoot(root);
    window.run();
    return 0;
}

} // namespace AureliaUI::MikoUI

