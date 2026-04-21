#pragma once
#include <windows.h>
#include <dwmapi.h>
#include <memory>
#include <vector>
#include <include/core/SkCanvas.h>
#include <include/core/SkBitmap.h>
#include <include/gui/Layout.hpp>
#include <include/gui/MenuBar.hpp>

namespace MochiUI {

struct MouseEvent {
    float x, y;
    bool pressed;
};

class Window {
public:
    Window(const std::string& title, int width, int height);
    virtual ~Window();

    void setTitleBarColor(SkColor color);
    void setDarkMode(bool enable);
    void enableMica(bool enable);
    
    void setMenuBar(std::unique_ptr<IMenuBar> bar);
    void setRoot(FlexNode::Ptr node) { root = node; }
    void run();

protected:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void onPaint();
    void onSize(int w, int h);
    void onMouseMove(float x, float y);
    void onMouseDown(float x, float y);

    HWND hwnd;
    SkBitmap bitmap;
    FlexNode::Ptr root;
    FlexNode::Ptr masterRoot;
    std::unique_ptr<IMenuBar> menuBar;
    int width, height;
};

// Simple Application State
class App {
public:
    static App& Get() {
        static App instance;
        return instance;
    }
    
    void init();
};

} // namespace MochiUI
