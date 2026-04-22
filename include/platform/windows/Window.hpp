#pragma once
#include <include/core/Window.hpp>
#include <windows.h>
#include <include/core/SkBitmap.h>

namespace MochiUI {

class Win32Window : public IWindow {
public:
    Win32Window(const std::string& title, int width, int height);
    ~Win32Window();

    void setTitle(const std::string& title) override;
    void setDarkMode(bool enable) override;
    void enableMica(bool enable) override;
    
    void setMenuBar(std::unique_ptr<IMenuBar> bar) override;
    void setRoot(FlexNode::Ptr node) override { root = node; }
    void run() override;
    
    void* getNativeHandle() const override { return (void*)hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void onPaint();
    void onSize(int w, int h);
    
    HWND hwnd;
    SkBitmap bitmap;
    FlexNode::Ptr root;
    FlexNode::Ptr masterRoot;
    std::unique_ptr<IMenuBar> menuBar;
    int width, height;
};

} // namespace MochiUI
