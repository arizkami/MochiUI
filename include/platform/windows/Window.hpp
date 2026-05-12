#pragma once
#include <core/Window.hpp>
#include <windows.h>
#include <d3d12.h>
#include <dcomp.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <vector>

class GrDirectContext;

#include <gui/OverlayNode.hpp>

namespace SphereUI {

struct FrameContext {
    Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
    sk_sp<SkSurface> surface;
};

class Win32Window : public IWindow, public IWindowHost {
public:
    Win32Window(const std::string& title, int width, int height);
    ~Win32Window();

    void setTitle(const std::string& title) override;
    void setDarkMode(bool enable) override;
    void enableMica(bool enable) override;
    void setSystemBackdrop(SystemBackdrop backdrop) override;
    void setTransparentBackground(bool enable) override;
    void setWindowMode(WindowMode mode) override;
    void setOpacity(float opacity) override;
    void setAlwaysOnTop(bool enable) override;
    void setCornerPreference(CornerPreference corner) override;
    void setShadow(bool enable) override;

    void setSize(int width, int height) override;
    void setPosition(int x, int y) override;
    void center() override;
    void setMinSize(int width, int height) override;
    int getWidth() const override { return width; }
    int getHeight() const override { return height; }
    float getDpiScale() const { return dpiScale; }

    void minimize() override;
    void maximize() override;
    void restore() override;
    void close() override;

    void startDrag() override;
    void setShellAppBar(ShellAppBarEdge edge, int thickness, int marginStart = 0, int marginEnd = 0) override;
    void clearShellAppBar() override;
    bool isShellAppBarRegistered() const override { return shellAppBarRegistered; }

    void setMenuBar(std::unique_ptr<IMenuBar> bar) override;
    void setRoot(FlexNode::Ptr node) override;
    void run() override;

    void* getNativeHandle() const override { return (void*)hwnd; }

    void requestRedraw() override {
        InvalidateRect(hwnd, NULL, FALSE);
    }

    void addOverlay(FlexNode::Ptr overlay) override {
        if (!overlayRoot) overlayRoot = std::make_shared<OverlayNode>();
        overlayRoot->addOverlay(overlay);
    }

    void removeOverlay(FlexNode::Ptr overlay) override {
        if (overlayRoot) overlayRoot->removeOverlay(overlay);
    }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void onPaint();
    void onSize(int pixelWidth, int pixelHeight);
    void onDpiChanged(UINT newDpi, const RECT* suggestedRect);

    bool initD3D12();
    bool createSwapChain();
    bool createHwndSwapChain(IDXGIFactory4* factory, DXGI_SWAP_CHAIN_DESC1 swapChainDesc);
    bool createCompositionSwapChain(IDXGIFactory4* factory, DXGI_SWAP_CHAIN_DESC1 swapChainDesc);
    void recreateSwapChain();
    void cleanupD3D12();
    void resizeBuffers(int pixelWidth, int pixelHeight);
    void updateDpi();
    int logicalToPixel(float value) const;
    float pixelToLogical(int value) const;
    float pixelToLogical(float value) const;
    void registerShellAppBar();
    void unregisterShellAppBar();
    void updateShellAppBarPosition();
    RECT getShellAppBarRect(HMONITOR monitor, int thicknessPx, int marginStartPx, int marginEndPx) const;
    void applyShellAppBarWindowStyle();
    void applyBackdrop();

    HWND hwnd;
    OverlayNode::Ptr overlayRoot;
    FlexNode::Ptr root; // Keep for compatibility if needed, but we use overlayRoot
    FlexNode::Ptr masterRoot;
    std::unique_ptr<IMenuBar> menuBar;
    int width, height;
    int pixelWidth = 0;
    int pixelHeight = 0;
    UINT dpi = 96;
    float dpiScale = 1.0f;

    // D3D12 resources
    Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
    Microsoft::WRL::ComPtr<IDCompositionDevice> dcompDevice;
    Microsoft::WRL::ComPtr<IDCompositionTarget> dcompTarget;
    Microsoft::WRL::ComPtr<IDCompositionVisual> dcompVisual;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    HANDLE fenceEvent = nullptr;
    uint64_t fenceValue = 0;

    static const int bufferCount = 2;
    int currentFrameIndex = 0;
    std::vector<FrameContext> frames;

    sk_sp<GrDirectContext> grContext;
    bool usingCompositionSwapChain = false;
    WindowMode currentMode = WindowMode::Windowed;
    WINDOWPLACEMENT wpPrev = { sizeof(wpPrev) };
    int minWidth = 0;
    int minHeight = 0;
    SystemBackdrop systemBackdrop = SystemBackdrop::None;
    bool transparentBackground = false;
    bool shellAppBarEnabled = false;
    bool shellAppBarRegistered = false;
    ShellAppBarEdge shellAppBarEdge = ShellAppBarEdge::Top;
    int shellAppBarThickness = 0;
    int shellAppBarMarginStart = 0;
    int shellAppBarMarginEnd = 0;
};

} // namespace SphereUI
