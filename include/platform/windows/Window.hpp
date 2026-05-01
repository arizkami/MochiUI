#pragma once
#include <core/Window.hpp>
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <vector>

class GrDirectContext;

#include <gui/OverlayNode.hpp>

namespace AureliaUI {

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
    void setWindowMode(WindowMode mode) override;

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
    void onSize(int w, int h);

    bool initD3D12();
    void cleanupD3D12();
    void resizeBuffers(int width, int height);

    HWND hwnd;
    OverlayNode::Ptr overlayRoot;
    FlexNode::Ptr root; // Keep for compatibility if needed, but we use overlayRoot
    FlexNode::Ptr masterRoot;
    std::unique_ptr<IMenuBar> menuBar;
    int width, height;

    // D3D12 resources
    Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    HANDLE fenceEvent = nullptr;
    uint64_t fenceValue = 0;

    static const int bufferCount = 2;
    int currentFrameIndex = 0;
    std::vector<FrameContext> frames;

    sk_sp<GrDirectContext> grContext;
    WindowMode currentMode = WindowMode::Windowed;
    WINDOWPLACEMENT wpPrev = { sizeof(wpPrev) };
};

} // namespace AureliaUI
