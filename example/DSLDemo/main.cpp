#include <AUKApplication.hpp>
#include <AUKGraphicInterface.hpp>
#include <AUKGraphicComponents.hpp>
#include <AUKDSL.hpp>

#include <windows.h>
#include <string>

using namespace AureliaUI;

#include <core/ResourceManager.hpp>
#include <DSLDemoResources.hpp>

// Wrap the YAML-driven navbar inside a full-page chrome that mirrors the
// dark colour scheme requested by the YAML so the tree has a sensible
// background to live against.
static FlexNode::Ptr BuildRoot() {
    auto root = FlexNode::Column();
    root->style.backgroundColor = AUKColor::Hex("#0b0f14");
    root->style.setWidthFull();
    root->style.setHeightFull();

    InitDSLDemoResources();
    const std::string ui = ResourceManager::getInstance().getResourceString("res://ui/container.yml");
    if (!ui.empty()) {
        if (auto tree = DSL::loadFromString(ui)) {
            root->addChild(tree);
            return root;
        }
    } else {
        auto err = std::make_shared<TextNode>("Failed to load res://ui/container.yml");
        err->color = AUKColor::Hex("#ff6b6b");
        err->fontSize = 14.0f;
        err->style.setPadding(16);
        root->addChild(err);
    }

    return root;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Application::getInstance().init();

    Win32Window window("AureliaKit DSL Demo", 1180, 720);
    window.enableMica(true);
    window.setDarkMode(true);
    window.setRoot(BuildRoot());
    window.run();
    return 0;
}
