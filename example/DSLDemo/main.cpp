#include <AUKApplication.hpp>
#include <AUKGraphicInterface.hpp>
#include <AUKGraphicComponents.hpp>
#include <AUKDSL.hpp>

#include <windows.h>
#include <filesystem>
#include <fstream>
#include <string>

using namespace AureliaUI;

// Locate `ui/navbar.yml` relative to the executable. CMake stages the demo's
// `ui/` folder next to the binary as a post-build step, so this resolves at
// runtime regardless of where the user launches it from.
static std::string ResolveNavbarPath() {
    namespace fs = std::filesystem;

    char exePath[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    fs::path exeDir = fs::path(exePath).parent_path();

    const fs::path candidates[] = {
        exeDir / "ui" / "navbar.yml",
        exeDir / "DSLDemo" / "ui" / "navbar.yml",
        exeDir.parent_path() / "example" / "DSLDemo" / "ui" / "navbar.yml",
        fs::current_path() / "example" / "DSLDemo" / "ui" / "navbar.yml",
    };
    for (const auto& p : candidates) {
        if (fs::exists(p)) return p.string();
    }
    return (exeDir / "ui" / "navbar.yml").string();
}

// Wrap the YAML-driven navbar inside a full-page chrome that mirrors the
// dark colour scheme requested by the YAML so the tree has a sensible
// background to live against.
static FlexNode::Ptr BuildRoot() {
    auto root = FlexNode::Column();
    root->style.backgroundColor = AUKColor::Hex("#0b0f14");
    root->style.setWidthFull();
    root->style.setHeightFull();

    const std::string yamlPath = ResolveNavbarPath();
    if (auto navbar = DSL::loadFromFile(yamlPath)) {
        root->addChild(navbar);
    } else {
        auto err = std::make_shared<TextNode>(
            "Failed to load DSL document: " + yamlPath);
        err->color    = AUKColor::Hex("#ff6b6b");
        err->fontSize = 14.0f;
        err->style.setPadding(16);
        root->addChild(err);
    }

    auto body = std::make_shared<TextNode>(
        "  Navbar above is built entirely from `ui/navbar.yml` via AUKDSL.");
    body->color    = AUKColor::Hex("#9ca3af");
    body->fontSize = 13.0f;
    body->style.setPadding(16);
    root->addChild(body);

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
