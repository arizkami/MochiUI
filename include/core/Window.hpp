#pragma once
#include <string>
#include <memory>
#include <core/IWindowHost.hpp>
#include <gui/Layout.hpp>
#include <gui/MenuBar.hpp>

namespace AureliaUI {

enum class WindowMode {
    Windowed,
    Borderless,
    Fullscreen
};

class IWindow {
public:
    virtual ~IWindow() = default;

    virtual void setTitle(const std::string& title) = 0;
    virtual void setDarkMode(bool enable) = 0;
    virtual void enableMica(bool enable) = 0;
    virtual void setWindowMode(WindowMode mode) = 0;

    virtual void setMenuBar(std::unique_ptr<IMenuBar> bar) = 0;
    virtual void setRoot(FlexNode::Ptr node) = 0;
    virtual void run() = 0;

    virtual void* getNativeHandle() const = 0;
};

} // namespace AureliaUI
