#pragma once
#include <string>
#include <memory>
#include <core/IWindowHost.hpp>
#include <gui/Layout.hpp>
#include <gui/MenuBar.hpp>

namespace SphereUI {

enum class WindowMode {
    Windowed,
    Borderless,
    Fullscreen
};

enum class CornerPreference {
    Default,    // system decides (rounded on Win11)
    None,       // square corners
    Round,      // large round corners
    RoundSmall, // small round corners
};

class IWindow {
public:
    virtual ~IWindow() = default;

    // Title / appearance
    virtual void setTitle(const std::string& title) = 0;
    virtual void setDarkMode(bool enable) = 0;
    virtual void enableMica(bool enable) = 0;
    virtual void setWindowMode(WindowMode mode) = 0;
    virtual void setOpacity(float opacity) = 0;
    virtual void setAlwaysOnTop(bool enable) = 0;

    // DWM visual effects (Windows 11)
    virtual void setCornerPreference(CornerPreference corner) = 0;
    virtual void setShadow(bool enable) = 0;

    // Size & position
    virtual void setSize(int width, int height) = 0;
    virtual void setPosition(int x, int y) = 0;
    virtual void center() = 0;
    virtual void setMinSize(int width, int height) = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;

    // Window state
    virtual void minimize() = 0;
    virtual void maximize() = 0;
    virtual void restore() = 0;
    virtual void close() = 0;

    // Borderless drag support
    virtual void startDrag() = 0;

    // UI tree / menu
    virtual void setMenuBar(std::unique_ptr<IMenuBar> bar) = 0;
    virtual void setRoot(FlexNode::Ptr node) = 0;
    virtual void run() = 0;

    virtual void* getNativeHandle() const = 0;
    virtual void requestRedraw() = 0;
};

} // namespace SphereUI
