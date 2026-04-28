#pragma once
#include <string>
#include <memory>
#include <include/core/IWindowHost.hpp>
#include <include/gui/Layout.hpp>
#include <include/gui/MenuBar.hpp>

namespace MochiUI {

class IWindow {
public:
    virtual ~IWindow() = default;

    virtual void setTitle(const std::string& title) = 0;
    virtual void setDarkMode(bool enable) = 0;
    virtual void enableMica(bool enable) = 0;
    
    virtual void setMenuBar(std::unique_ptr<IMenuBar> bar) = 0;
    virtual void setRoot(FlexNode::Ptr node) = 0;
    virtual void run() = 0;
    
    virtual void* getNativeHandle() const = 0;
};

} // namespace MochiUI
