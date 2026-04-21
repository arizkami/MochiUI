#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <include/gui/Layout.hpp>

namespace MochiUI {

enum class MenuBackend {
    Win32, // Native Windows Menu
    Skia   // Custom Skia-rendered Menu
};

struct MenuItem {
    std::string label;
    int id;
    std::function<void()> action;
    std::vector<MenuItem> subItems;
};

class IMenuBar {
public:
    virtual ~IMenuBar() = default;
    virtual void addMenu(const std::string& label, const std::vector<MenuItem>& items) = 0;
    virtual void attach(void* windowHandle) = 0; // For Win32
    virtual FlexNode::Ptr getLayoutNode() = 0;   // For Skia
};

class MenuBarFactory {
public:
    static std::unique_ptr<IMenuBar> Create(MenuBackend backend);
};

} // namespace MochiUI
