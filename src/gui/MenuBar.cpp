#include "MenuBar.hpp"
#include "Components.hpp"
#include "Theme.hpp"
#include <windows.h>
#include <map>

namespace MochiUI {

// --- Win32 Backend Implementation ---
class Win32MenuBar : public IMenuBar {
    HMENU hMenuBar;
    std::map<int, std::function<void()>> actions;

public:
    Win32MenuBar() {
        hMenuBar = CreateMenu();
    }

    void addMenu(const std::string& label, const std::vector<MenuItem>& items) override {
        HMENU hMenu = CreatePopupMenu();
        for (const auto& item : items) {
            AppendMenuA(hMenu, MF_STRING, item.id, item.label.c_str());
            if (item.action) {
                actions[item.id] = item.action;
            }
        }
        AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hMenu, label.c_str());
    }

    void attach(void* windowHandle) override {
        SetMenu((HWND)windowHandle, hMenuBar);
    }

    FlexNode::Ptr getLayoutNode() override { return nullptr; }
};

// --- Skia Backend Implementation ---
class SkiaMenuBar : public IMenuBar {
    FlexNode::Ptr rootNode;

public:
    SkiaMenuBar() {
        rootNode = FlexNode::Row();
        rootNode->style.height = 18;
        rootNode->style.heightMode = SizingMode::Fixed;
        rootNode->style.widthMode = SizingMode::Flex;
        rootNode->style.backgroundColor = Theme::MenuBar;
        rootNode->style.padding = 2;
        rootNode->style.gap = 5;
    }

    void addMenu(const std::string& label, const std::vector<MenuItem>& items) override {
        auto menuBtn = std::make_shared<TextNode>();
        menuBtn->text = label;
        menuBtn->style.widthMode = SizingMode::Hug;
        menuBtn->style.heightMode = SizingMode::Flex;
        menuBtn->style.padding = 4;
        menuBtn->style.borderRadius = 2;
        menuBtn->color = Theme::TextPrimary;
        menuBtn->fontSize = 12;
        
        menuBtn->onClick = [label]() {
            OutputDebugStringA(("Menu Clicked: " + label + "\n").c_str());
        };

        rootNode->addChild(menuBtn);
    }

    void attach(void* windowHandle) override { }
    
    FlexNode::Ptr getLayoutNode() override {
        return rootNode;
    }
};

std::unique_ptr<IMenuBar> MenuBarFactory::Create(MenuBackend backend) {
    if (backend == MenuBackend::Win32) return std::make_unique<Win32MenuBar>();
    return std::make_unique<SkiaMenuBar>();
}

} // namespace MochiUI
