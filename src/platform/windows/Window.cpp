#include <platform/windows/Window.hpp>
#include <core/events/Win32Input.hpp>
#include <core/events/FlexRootDispatch.hpp>
#include <utils/Misc/ThemeSwitcher.hpp>
#include <include/core/SkCanvas.h>
#include <include/core/SkColorSpace.h>
#include <include/core/SkSurfaceProps.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/gpu/ganesh/d3d/GrD3DBackendContext.h>
#include <include/gpu/ganesh/d3d/GrD3DDirectContext.h>
#include <include/gpu/ganesh/d3d/GrD3DTypes.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/ganesh/d3d/GrD3DBackendSurface.h>
#include <include/gpu/ganesh/GrBackendSurface.h>
#include <dwmapi.h>
#include <windowsx.h>

namespace AureliaUI {

// Aero-borderless style: WS_CAPTION kept so DWM provides shadow, snap, and
// window animations; WM_NCCALCSIZE strips the visual chrome at zero cost.
static constexpr DWORD kAeroBorderlessStyle =
    WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

static bool compositionEnabled() {
    BOOL enabled = FALSE;
    return DwmIsCompositionEnabled(&enabled) == S_OK && enabled;
}

class SimpleD3DAlloc : public GrD3DAlloc {};

class SimpleD3DMemoryAllocator : public GrD3DMemoryAllocator {
public:
    SimpleD3DMemoryAllocator(ID3D12Device* device) : fDevice(device) {
        fDevice->AddRef();
    }
    ~SimpleD3DMemoryAllocator() override {
        fDevice->Release();
    }

    gr_cp<ID3D12Resource> createResource(D3D12_HEAP_TYPE heapType, const D3D12_RESOURCE_DESC* desc,
                                         D3D12_RESOURCE_STATES initialResourceState,
                                         sk_sp<GrD3DAlloc>* allocation,
                                         const D3D12_CLEAR_VALUE* clearValue) override {
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = heapType;

        ID3D12Resource* resource = nullptr;
        HRESULT hr = fDevice->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            desc,
            initialResourceState,
            clearValue,
            IID_PPV_ARGS(&resource)
        );
        if (FAILED(hr)) return nullptr;

        *allocation = sk_sp<GrD3DAlloc>(new SimpleD3DAlloc());
        return gr_cp<ID3D12Resource>(resource);
    }

    gr_cp<ID3D12Resource> createAliasingResource(sk_sp<GrD3DAlloc>& allocation,
                                                 uint64_t localOffset,
                                                 const D3D12_RESOURCE_DESC*,
                                                 D3D12_RESOURCE_STATES initialResourceState,
                                                 const D3D12_CLEAR_VALUE*) override {
        return nullptr;
    }

private:
    ID3D12Device* fDevice;
};

Win32Window::Win32Window(const std::string& title, int width, int height) : width(width), height(height) {
    const wchar_t CLASS_NAME[] = L"AureliaUIWindow";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = Win32Window::WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    wc.hIcon = LoadIconW(wc.hInstance, (LPCWSTR)MAKEINTRESOURCE(101));
    wc.hIconSm = LoadIconW(wc.hInstance, (LPCWSTR)MAKEINTRESOURCE(101));

    RegisterClassExW(&wc);

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, NULL, 0);
    std::wstring wtitle(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wtitle[0], size_needed);
    // Remove the extra null terminator added by wtitle resizing
    if (!wtitle.empty() && wtitle.back() == L'\0') {
        wtitle.pop_back();
    }

    hwnd = CreateWindowExW(
        0, CLASS_NAME, wtitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, wc.hInstance, this
    );
    if (hwnd) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

        // Ensure icon is set for the window and taskbar
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)wc.hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)wc.hIconSm);

        auto& switcher = ThemeSwitcher::getInstance();
        bool isDark = (switcher.getCurrentTheme() == ThemeType::Auto)
                      ? switcher.isWindowsInDarkMode()
                      : (switcher.getCurrentTheme() == ThemeType::Dark);
        setDarkMode(isDark);

        initD3D12();
    }
}

Win32Window::~Win32Window() {
    cleanupD3D12();
}

bool Win32Window::initD3D12() {
    HRESULT hr;

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif
    hr = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return false;

    Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &hardwareAdapter); ++adapterIndex) {
        DXGI_ADAPTER_DESC1 desc;
        hardwareAdapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
        if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
            break;
        }
    }

    hr = D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice));
    if (FAILED(hr)) return false;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    hr = d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
    if (FAILED(hr)) return false;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    hr = factory->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    );
    if (FAILED(hr)) return false;

    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    swapChain1.As(&swapChain);
    currentFrameIndex = swapChain->GetCurrentBackBufferIndex();

    hr = d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(hr)) return false;
    fenceValue = 1;
    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    GrD3DBackendContext backendContext;
    backendContext.fAdapter.retain(hardwareAdapter.Get());
    backendContext.fDevice.retain(d3dDevice.Get());
    backendContext.fQueue.retain(commandQueue.Get());
    backendContext.fMemoryAllocator = sk_make_sp<SimpleD3DMemoryAllocator>(d3dDevice.Get());

    GrContextOptions options;
    grContext = GrDirectContexts::MakeD3D(backendContext, options);

    if (!grContext) return false;

    resizeBuffers(width, height);
    return true;
}

void Win32Window::cleanupD3D12() {
    if (grContext) {
        grContext->flush();
        grContext->submit(GrSyncCpu::kYes);
    }

    // Clear surfaces first before context dies
    frames.clear();

    if (grContext) {
        grContext->releaseResourcesAndAbandonContext();
        grContext.reset();
    }

    // Wait for the GPU to finish
    if (commandQueue && fence && fenceEvent) {
        const uint64_t fenceVal = fenceValue;
        commandQueue->Signal(fence.Get(), fenceVal);
        fenceValue++;
        if (fence->GetCompletedValue() < fenceVal) {
            fence->SetEventOnCompletion(fenceVal, fenceEvent);
            WaitForSingleObject(fenceEvent, INFINITE);
        }
    }

    if (fenceEvent) {
        CloseHandle(fenceEvent);
        fenceEvent = nullptr;
    }

    fence.Reset();
    swapChain.Reset();
    commandQueue.Reset();
    d3dDevice.Reset();
}

void Win32Window::resizeBuffers(int w, int h) {
    if (!swapChain || !grContext) return;

    // Flush and wait for GPU to finish all work before releasing back buffers
    grContext->flush();
    grContext->submit(GrSyncCpu::kYes);

    if (commandQueue && fence && fenceEvent) {
        const uint64_t fenceVal = fenceValue;
        commandQueue->Signal(fence.Get(), fenceVal);
        fenceValue++;
        if (fence->GetCompletedValue() < fenceVal) {
            fence->SetEventOnCompletion(fenceVal, fenceEvent);
            WaitForSingleObject(fenceEvent, INFINITE);
        }
    }

    // Must release all Skia surfaces and D3D12 back buffers before resizing
    frames.clear();

    HRESULT hr = swapChain->ResizeBuffers(bufferCount, w, h, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    if (FAILED(hr)) return;

    frames.resize(bufferCount);
    currentFrameIndex = swapChain->GetCurrentBackBufferIndex();

    for (int i = 0; i < bufferCount; ++i) {
        hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&frames[i].backBuffer));
        if (FAILED(hr)) continue;

        GrD3DTextureResourceInfo info(frames[i].backBuffer.Get(),
                                      nullptr,
                                      D3D12_RESOURCE_STATE_PRESENT,
                                      DXGI_FORMAT_R8G8B8A8_UNORM,
                                      1,
                                      1,
                                      0);

        GrBackendRenderTarget backendRT = GrBackendRenderTargets::MakeD3D(w, h, info);
        SkSurfaceProps props(0, kRGB_H_SkPixelGeometry);
        frames[i].surface = SkSurfaces::WrapBackendRenderTarget(
            grContext.get(), backendRT, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType, nullptr, &props);
    }
}

void Win32Window::setTitle(const std::string& title) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, NULL, 0);
    std::wstring wtitle(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wtitle[0], size_needed);
    SetWindowTextW(hwnd, wtitle.c_str());
}

void Win32Window::setDarkMode(bool enable) {
    BOOL value = enable ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
}

void Win32Window::enableMica(bool enable) {
    int value = enable ? 2 : 0;
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &value, sizeof(value));
}

void Win32Window::setWindowMode(WindowMode mode) {
    if (currentMode == mode) return;

    WindowMode prevMode = currentMode;
    // Set currentMode BEFORE any SetWindowPos/ShowWindow so that WM_NCCALCSIZE
    // (fired synchronously by SWP_FRAMECHANGED) sees the new mode immediately.
    currentMode = mode;

    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

    if (mode == WindowMode::Fullscreen) {
        if (prevMode == WindowMode::Windowed) {
            GetWindowPlacement(hwnd, &wpPrev);
        }
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hwnd, HWND_TOP,
                         mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        }
    } else if (mode == WindowMode::Windowed) {
        SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        MARGINS margins = {0, 0, 0, 0};
        DwmExtendFrameIntoClientArea(hwnd, &margins);
        if (prevMode == WindowMode::Fullscreen || prevMode == WindowMode::Borderless) {
            SetWindowPlacement(hwnd, &wpPrev);
        }
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        ShowWindow(hwnd, SW_SHOW);
    } else if (mode == WindowMode::Borderless) {
        if (prevMode == WindowMode::Windowed) {
            GetWindowPlacement(hwnd, &wpPrev);
        }
        SetWindowLongPtr(hwnd, GWL_STYLE, kAeroBorderlessStyle);
        if (compositionEnabled()) {
            MARGINS margins = {1, 1, 1, 1};
            DwmExtendFrameIntoClientArea(hwnd, &margins);
        }
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        ShowWindow(hwnd, SW_SHOW);
    }
}

void Win32Window::setOpacity(float opacity) {
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (opacity < 1.0f) {
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hwnd, 0, (BYTE)(opacity * 255), LWA_ALPHA);
    } else {
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_LAYERED);
    }
}

void Win32Window::setAlwaysOnTop(bool enable) {
    SetWindowPos(hwnd, enable ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void Win32Window::setCornerPreference(CornerPreference corner) {
    // DWMWA_WINDOW_CORNER_PREFERENCE = 33, available since Windows 11 Build 22000
    static constexpr DWORD DWMWA_WINDOW_CORNER_PREFERENCE = 33;
    DWORD value = 0; // DWMWCP_DEFAULT
    switch (corner) {
        case CornerPreference::None:       value = 1; break; // DWMWCP_DONOTROUND
        case CornerPreference::Round:      value = 2; break; // DWMWCP_ROUND
        case CornerPreference::RoundSmall: value = 3; break; // DWMWCP_ROUNDSMALL
        default:                           value = 0; break; // DWMWCP_DEFAULT
    }
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &value, sizeof(value));
}

void Win32Window::setShadow(bool enable) {
    if (compositionEnabled()) {
        MARGINS margins = enable ? MARGINS{1, 1, 1, 1} : MARGINS{0, 0, 0, 0};
        DwmExtendFrameIntoClientArea(hwnd, &margins);
    }
}

void Win32Window::setSize(int w, int h) {
    SetWindowPos(hwnd, NULL, 0, 0, w, h,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}

void Win32Window::setPosition(int x, int y) {
    SetWindowPos(hwnd, NULL, x, y, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}

void Win32Window::center() {
    HMONITOR mon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfo(mon, &mi);
    RECT wr;
    GetWindowRect(hwnd, &wr);
    int ww = wr.right - wr.left;
    int wh = wr.bottom - wr.top;
    int mx = mi.rcWork.left + (mi.rcWork.right  - mi.rcWork.left - ww) / 2;
    int my = mi.rcWork.top  + (mi.rcWork.bottom - mi.rcWork.top  - wh) / 2;
    SetWindowPos(hwnd, NULL, mx, my, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}

void Win32Window::setMinSize(int w, int h) {
    minWidth  = w;
    minHeight = h;
}

void Win32Window::minimize() {
    ShowWindow(hwnd, SW_MINIMIZE);
}

void Win32Window::maximize() {
    ShowWindow(hwnd, SW_MAXIMIZE);
}

void Win32Window::restore() {
    ShowWindow(hwnd, SW_RESTORE);
}

void Win32Window::close() {
    PostMessage(hwnd, WM_CLOSE, 0, 0);
}

void Win32Window::startDrag() {
    ReleaseCapture();
    SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
}

void Win32Window::setMenuBar(std::unique_ptr<IMenuBar> bar) {
    menuBar = std::move(bar);
    if (menuBar) {
        menuBar->attach(hwnd);
    }
}

void Win32Window::setRoot(FlexNode::Ptr node) {
    root = node;
    if (!overlayRoot) {
        overlayRoot = std::make_shared<OverlayNode>();
        overlayRoot->setWindowHost(this);
    }
    overlayRoot->setMainContent(node);
}

void Win32Window::onSize(int w, int h) {
    if (w == 0 || h == 0) return;
    width = w;
    height = h;

    resizeBuffers(w, h);

    if (overlayRoot) {
        if (menuBar && menuBar->getLayoutNode()) {
            if (!masterRoot) {
                masterRoot = FlexNode::Column();
                masterRoot->setWindowHost(this);
                masterRoot->style.setWidthPercent(100.0f);
                masterRoot->style.setHeightPercent(100.0f);

                auto contentWrapper = FlexNode::Column();
                contentWrapper->style.setFlex(1.0f);
                contentWrapper->addChild(menuBar->getLayoutNode());
                if (root) contentWrapper->addChild(root);

                overlayRoot->setMainContent(contentWrapper);
            }
        } else {
            if (root) overlayRoot->setMainContent(root);
        }
        overlayRoot->calculateLayout(SkRect::MakeWH((float)w, (float)h));
    }

    // Synchronous paint to reduce jitter during resize
    onPaint();
}

void Win32Window::onPaint() {
    if (!overlayRoot || !grContext || frames.empty()) return;

    currentFrameIndex = swapChain->GetCurrentBackBufferIndex();
    if (currentFrameIndex >= frames.size()) return;

    auto surface = frames[currentFrameIndex].surface;
    if (!surface) return;

    SkCanvas* canvas = surface->getCanvas();
    if (!canvas) return;

    if (overlayRoot->dirtyLayout) {
        overlayRoot->calculateLayout(SkRect::MakeWH((float)width, (float)height));
    }

    canvas->clear(SK_ColorTRANSPARENT);

    overlayRoot->draw(canvas);

    grContext->flush(surface.get());
    grContext->submit();

    // Transition resource to present state
    GrBackendRenderTarget backendRT = SkSurfaces::GetBackendRenderTarget(surface.get(), SkSurface::kFlushRead_BackendHandleAccess);
    if (backendRT.isValid()) {
        GrBackendRenderTargets::SetD3DResourceState(&backendRT, D3D12_RESOURCE_STATE_PRESENT);
    }

    bool needsMoreRedraw = overlayRoot->needsRedraw();

    // Present without VSync so the UI thread is never blocked waiting for
    // vblank (~16 ms). Frame pacing is handled by the MsgWaitForMultipleObjects
    // call in run(). DXGI_PRESENT_ALLOW_TEARING is not requested here so the
    // driver may still sync internally when the GPU is ahead.
    swapChain->Present(0, 0);

    // Signal the fence for this frame and wait only if the GPU is more than
    // one frame behind (prevents overwriting a buffer still in flight).
    const uint64_t fenceVal = fenceValue;
    commandQueue->Signal(fence.Get(), fenceVal);
    fenceValue++;
    if (fence->GetCompletedValue() < fenceVal - 1) {
        fence->SetEventOnCompletion(fenceVal - 1, fenceEvent);
        WaitForSingleObject(fenceEvent, 8); // max 8 ms — never block the UI thread long
    }

    ValidateRect(hwnd, NULL);
}

void Win32Window::run() {
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    RECT rect;
    GetClientRect(hwnd, &rect);
    onSize(rect.right - rect.left, rect.bottom - rect.top);

    MSG msg = {};
    while (true) {
        // Drain ALL pending messages before rendering so rapid click sequences
        // are fully processed without being interleaved with GPU fence waits.
        bool hadMessage = false;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            hadMessage = true;
        }

        // Idle: pace animation redraws or sleep until the next input event.
        if (overlayRoot && overlayRoot->needsRedraw()) {
            // Use MsgWaitForMultipleObjects so input always wakes us instantly
            // even during animation; ~120fps cap (8ms) prevents busy-spinning.
            MsgWaitForMultipleObjects(0, nullptr, FALSE, 8, QS_ALLINPUT);
            InvalidateRect(hwnd, NULL, FALSE);
        } else if (!hadMessage) {
            WaitMessage();
        }
    }
}

LRESULT CALLBACK Win32Window::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Win32Window* win = (Win32Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    FlexNode::Ptr effectiveRoot = win ? win->overlayRoot : nullptr;

    switch (msg) {
        case WM_NCCALCSIZE:
            // When borderless, tell Windows the client area = entire window rect.
            // This removes the native title bar / borders visually while DWM still
            // recognises the window as having WS_CAPTION (shadow + snap intact).
            if (wp == TRUE && win && win->currentMode == WindowMode::Borderless) {
                auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(lp);
                // When maximised the window rect extends beyond the monitor edges;
                // clip to the work area so content isn't hidden behind the taskbar.
                if (IsZoomed(hwnd)) {
                    MONITORINFO mi = { sizeof(mi) };
                    if (GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &mi))
                        params.rgrc[0] = mi.rcWork;
                }
                return 0;
            }
            break;

        case WM_NCHITTEST: {
            // For borderless windows provide resize hit-zones at the edges so that
            // aero-snap and manual resizing still work without native chrome.
            if (win && win->currentMode == WindowMode::Borderless) {
                RECT wr;
                GetWindowRect(hwnd, &wr);
                POINT cur = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
                const int bx = GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
                const int by = GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
                const int L = (cur.x <  wr.left   + bx);
                const int R = (cur.x >= wr.right  - bx);
                const int T = (cur.y <  wr.top    + by);
                const int B = (cur.y >= wr.bottom - by);
                switch (L | (R << 1) | (T << 2) | (B << 3)) {
                    case 0b0001: return HTLEFT;
                    case 0b0010: return HTRIGHT;
                    case 0b0100: return HTTOP;
                    case 0b1000: return HTBOTTOM;
                    case 0b0101: return HTTOPLEFT;
                    case 0b0110: return HTTOPRIGHT;
                    case 0b1001: return HTBOTTOMLEFT;
                    case 0b1010: return HTBOTTOMRIGHT;
                }
                return HTCLIENT;
            }
            break;
        }

        case WM_NCACTIVATE:
            // Without DWM composition the default handler would briefly repaint the
            // native frame on activation; return 1 to suppress that artefact.
            if (win && win->currentMode == WindowMode::Borderless && !compositionEnabled())
                return 1;
            break;

        case WM_ERASEBKGND:
            return 1; // Prevent flicker
        case WM_MOUSEMOVE:
            if (win && effectiveRoot) {
                using namespace events;
                PointerEvent e = pointerEventFromClient(lp, pointerButtonsFromKeyState());
                if (dispatchPointerMove(*effectiveRoot, e)) {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        case WM_LBUTTONDBLCLK: // fall-through: treat double-click as a regular press
        case WM_LBUTTONDOWN:
            if (win && effectiveRoot) {
                using namespace events;
                PointerButton btns = pointerButtonsFromKeyState() | PointerButton::Primary;
                PointerEvent e = pointerEventFromClient(lp, btns, PointerButton::Primary, true);
                if (msg == WM_LBUTTONDBLCLK) e.clickCount = 2;
                if (dispatchPointerPrimaryDown(*effectiveRoot, e)) {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        case WM_LBUTTONUP:
            if (win && effectiveRoot) {
                using namespace events;
                PointerEvent e = pointerEventFromClient(
                    lp, pointerButtonsFromKeyState(), PointerButton::Primary, false);
                dispatchPointerPrimaryUp(*effectiveRoot, e);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        case WM_RBUTTONDOWN:
            if (win && effectiveRoot) {
                using namespace events;
                PointerButton btns = pointerButtonsFromKeyState() | PointerButton::Secondary;
                PointerEvent e = pointerEventFromClient(lp, btns, PointerButton::Secondary, true);
                if (dispatchPointerSecondaryDown(*effectiveRoot, e)) {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        case WM_MOUSEWHEEL:
            if (win && effectiveRoot) {
                using namespace events;
                WheelEvent e = wheelEventFromMouseWheel(hwnd, wp, lp);
                if (dispatchWheel(*effectiveRoot, e)) {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        case WM_KEYDOWN:
            if (win && effectiveRoot) {
                using namespace events;
                KeyEvent e = keyEventFromKeyDown(wp, lp);
                if (dispatchKeyDown(*effectiveRoot, e)) {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        case WM_CHAR:
            if (win && effectiveRoot) {
                using namespace events;
                TextEvent e = textEventFromWmChar(wp);
                if (dispatchTextInput(*effectiveRoot, e)) {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;

        case WM_SETCURSOR:
            if (win && LOWORD(lp) == HTCLIENT) {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(hwnd, &pt);

                if (win->overlayRoot) {
                    auto node = win->overlayRoot->findNodeAt((float)pt.x, (float)pt.y);
                    if (node) {
                        LPCWSTR cursorId = (LPCWSTR)IDC_ARROW;
                        switch (node->style.cursorType) {
                            case Cursor::Hand:   cursorId = (LPCWSTR)IDC_HAND; break;
                            case Cursor::IBeam:  cursorId = (LPCWSTR)IDC_IBEAM; break;
                            case Cursor::SizeNS: cursorId = (LPCWSTR)IDC_SIZENS; break;
                            case Cursor::SizeWE: cursorId = (LPCWSTR)IDC_SIZEWE; break;
                            default:             cursorId = (LPCWSTR)IDC_ARROW; break;
                        }
                        SetCursor(LoadCursorW(NULL, cursorId));
                        return TRUE;
                    }
                }
            }
            break;

        case WM_ENTERSIZEMOVE:
            SetTimer(hwnd, 1, 16, NULL); // ~60fps during drag/resize modal loop
            return 0;
        case WM_EXITSIZEMOVE:
            KillTimer(hwnd, 1);
            return 0;
        case WM_TIMER:
            if (wp == 1) InvalidateRect(hwnd, NULL, FALSE);
            return 0;

        case WM_GETMINMAXINFO:
            if (win && (win->minWidth > 0 || win->minHeight > 0)) {
                auto* mmi = reinterpret_cast<MINMAXINFO*>(lp);
                if (win->minWidth  > 0) mmi->ptMinTrackSize.x = win->minWidth;
                if (win->minHeight > 0) mmi->ptMinTrackSize.y = win->minHeight;
                return 0;
            }
            break;

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
    return DefWindowProcW(hwnd, msg, wp, lp);
}

} // namespace AureliaUI
