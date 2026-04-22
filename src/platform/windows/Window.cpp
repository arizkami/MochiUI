#include <include/platform/windows/Window.hpp>
#include <include/utils/Misc/ThemeSwitcher.hpp>
#include <include/core/SkCanvas.h>
#include <dwmapi.h>

namespace MochiUI {

Win32Window::Win32Window(const std::string& title, int width, int height) : width(width), height(height) {
    const char CLASS_NAME[] = "MochiUIWindow";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = Win32Window::WndProc;
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
        
        auto& switcher = ThemeSwitcher::getInstance();
        bool isDark = (switcher.getCurrentTheme() == ThemeType::Auto) 
                      ? switcher.isWindowsInDarkMode() 
                      : (switcher.getCurrentTheme() == ThemeType::Dark);
        setDarkMode(isDark);
    }
}

Win32Window::~Win32Window() {}

void Win32Window::setTitle(const std::string& title) {
    SetWindowTextA(hwnd, title.c_str());
}

void Win32Window::setDarkMode(bool enable) {
    BOOL value = enable ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
}

void Win32Window::enableMica(bool enable) {
    int value = enable ? 2 : 0; 
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &value, sizeof(value));
}

void Win32Window::setMenuBar(std::unique_ptr<IMenuBar> bar) {
    menuBar = std::move(bar);
    if (menuBar) {
        menuBar->attach(hwnd);
    }
}

void Win32Window::onSize(int w, int h) {
    width = w;
    height = h;
    bitmap.allocN32Pixels(w, h);

    if (root) {
        if (menuBar && menuBar->getLayoutNode()) {
            masterRoot = FlexNode::Column();
            masterRoot->style.widthMode = SizingMode::Flex;
            masterRoot->style.heightMode = SizingMode::Flex;
            
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

void Win32Window::onPaint() {
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

void Win32Window::run() {
    ShowWindow(hwnd, SW_SHOW);
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK Win32Window::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Win32Window* win = (Win32Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
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
        case WM_MOUSEWHEEL:
            if (win && effectiveRoot) {
                POINT pt = { LOWORD(lp), HIWORD(lp) };
                ScreenToClient(hwnd, &pt);
                float delta = GET_WHEEL_DELTA_WPARAM(wp) / (float)WHEEL_DELTA;
                if (effectiveRoot->onMouseWheel((float)pt.x, (float)pt.y, delta)) {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
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

} // namespace MochiUI
