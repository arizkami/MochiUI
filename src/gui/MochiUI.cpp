#include <include/gui/MochiUI.h>
#include <include/utils/FontManager/FontMgr.hpp>
#include <include/utils/Misc/ThemeSwitcher.hpp>
#include <include/core/SkCanvas.h>
#include <include/core/SkTypeface.h>
#include <include/core/SkFontMgr.h>
#include <include/ports/SkTypeface_win.h>

namespace MochiUI {

Window::Window(const std::string& title, int width, int height) : width(width), height(height) {
    const char CLASS_NAME[] = "MochiUIWindow";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = Window::WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        0, CLASS_NAME, title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, wc.hInstance, this
    );

    if (hwnd) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
        setDarkMode(true); // Default to dark mode
    }
}

Window::~Window() {}

void Window::setDarkMode(bool enable) {
    BOOL value = enable ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
}

void Window::enableMica(bool enable) {
    int value = enable ? 2 : 0; // 2 = Mica, 4 = MicaAlt
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &value, sizeof(value));
}

void Window::setMenuBar(std::unique_ptr<IMenuBar> bar) {
    menuBar = std::move(bar);
    if (menuBar) {
        menuBar->attach(hwnd);
    }
}

void Window::onSize(int w, int h) {
    width = w;
    height = h;
    bitmap.allocN32Pixels(w, h);

    if (root) {
        if (menuBar && menuBar->getLayoutNode()) {
            masterRoot = FlexNode::Column();
            masterRoot->style.widthMode = SizingMode::Flex;
            masterRoot->style.heightMode = SizingMode::Flex;
            
            // Fix: set flex to 1.0 so main content takes remaining height
            root->style.flex = 1.0f;
            root->style.heightMode = SizingMode::Flex;
            
            masterRoot->addChild(menuBar->getLayoutNode());
            masterRoot->addChild(root);
            masterRoot->calculateLayout(SkRect::MakeWH((float)w, (float)h));
        } else {
            masterRoot = nullptr;
            root->calculateLayout(SkRect::MakeWH((float)w, (float)h));
        }
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

void Window::onPaint() {
    if (!root || !bitmap.getPixels()) return;

    SkCanvas canvas(bitmap);
    if (masterRoot) {
        masterRoot->draw(&canvas);
    } else {
        root->draw(&canvas);
    }

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bitmap.width();
    bmi.bmiHeader.biHeight = -bitmap.height();
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    StretchDIBits(hdc, 0, 0, bitmap.width(), bitmap.height(), 
                  0, 0, bitmap.width(), bitmap.height(),
                  bitmap.getPixels(), &bmi, DIB_RGB_COLORS, SRCCOPY);
                  
    EndPaint(hwnd, &ps);
}

void Window::run() {
    ShowWindow(hwnd, SW_SHOW);
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Window* win = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    FlexNode::Ptr effectiveRoot = nullptr;
    if (win) {
        effectiveRoot = win->masterRoot ? win->masterRoot : win->root;
    }

    switch (msg) {
        case WM_MOUSEMOVE:
            if (win && effectiveRoot) {
                if (effectiveRoot->onMouseMove((float)LOWORD(lp), (float)HIWORD(lp))) {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        case WM_LBUTTONDOWN:
            if (win && effectiveRoot) {
                if (effectiveRoot->onMouseDown((float)LOWORD(lp), (float)HIWORD(lp))) {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        case WM_LBUTTONUP:
            if (win && effectiveRoot) {
                effectiveRoot->onMouseUp((float)LOWORD(lp), (float)HIWORD(lp));
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        case WM_SIZE:
            if (win) win->onSize(LOWORD(lp), HIWORD(lp));
            return 0;
        case WM_PAINT:
            if (win) win->onPaint();
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

void App::init() {
    FontManager::getInstance().initialize();
    
    // Initialize theme system - auto-detect Windows theme by default
    ThemeSwitcher::getInstance().applyTheme();
}

} // namespace MochiUI
