#include <platform/windows/Window.hpp>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <dcomp.h>
#include <algorithm>

namespace SphereUI {

bool Win32Window::wantsCompositionSwapChain() const {
    return transparentBackground || shellAppBarEnabled || systemBackdrop != SystemBackdrop::None;
}

DXGI_FORMAT Win32Window::swapChainFormat() const {
    return usingCompositionSwapChain ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
}

SkColorType Win32Window::swapChainColorType() const {
    return usingCompositionSwapChain ? kBGRA_8888_SkColorType : kRGBA_8888_SkColorType;
}

void Win32Window::applyCompositionWindowStyle() {
    if (!hwnd) return;

    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (wantsCompositionSwapChain()) {
        exStyle |= WS_EX_NOREDIRECTIONBITMAP;
    } else {
        exStyle &= ~WS_EX_NOREDIRECTIONBITMAP;
    }
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
}

bool Win32Window::createSwapChain() {
    if (!commandQueue || !hwnd) return false;

    applyCompositionWindowStyle();

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    if (FAILED(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory)))) return false;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.Width = std::max(1, pixelWidth);
    swapChainDesc.Height = std::max(1, pixelHeight);
    swapChainDesc.Format = wantsCompositionSwapChain()
        ? DXGI_FORMAT_B8G8R8A8_UNORM
        : DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = wantsCompositionSwapChain()
        ? DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
        : DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.AlphaMode = wantsCompositionSwapChain()
        ? DXGI_ALPHA_MODE_PREMULTIPLIED
        : DXGI_ALPHA_MODE_IGNORE;

    if (wantsCompositionSwapChain() && createCompositionSwapChain(factory.Get(), swapChainDesc)) {
        factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
        currentFrameIndex = swapChain->GetCurrentBackBufferIndex();
        usingCompositionSwapChain = true;
        return true;
    }

    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_NOREDIRECTIONBITMAP);
    if (!createHwndSwapChain(factory.Get(), swapChainDesc)) return false;

    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    currentFrameIndex = swapChain->GetCurrentBackBufferIndex();
    usingCompositionSwapChain = false;
    return true;
}

bool Win32Window::createHwndSwapChain(IDXGIFactory4* factory, DXGI_SWAP_CHAIN_DESC1 swapChainDesc) {
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    HRESULT hr = factory->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    );
    if (FAILED(hr)) return false;

    dcompVisual.Reset();
    dcompTarget.Reset();
    dcompDevice.Reset();
    swapChain1.As(&swapChain);
    return swapChain != nullptr;
}

bool Win32Window::createCompositionSwapChain(IDXGIFactory4* factory, DXGI_SWAP_CHAIN_DESC1 swapChainDesc) {
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    HRESULT hr = factory->CreateSwapChainForComposition(
        commandQueue.Get(),
        &swapChainDesc,
        nullptr,
        &swapChain1
    );
    if (FAILED(hr)) return false;

    Microsoft::WRL::ComPtr<IDCompositionDevice> nextDevice;
    Microsoft::WRL::ComPtr<IDCompositionTarget> nextTarget;
    Microsoft::WRL::ComPtr<IDCompositionVisual> nextVisual;

    hr = DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&nextDevice));
    if (FAILED(hr)) return false;
    hr = nextDevice->CreateTargetForHwnd(hwnd, TRUE, &nextTarget);
    if (FAILED(hr)) return false;
    hr = nextDevice->CreateVisual(&nextVisual);
    if (FAILED(hr)) return false;
    hr = nextVisual->SetContent(swapChain1.Get());
    if (FAILED(hr)) return false;
    hr = nextTarget->SetRoot(nextVisual.Get());
    if (FAILED(hr)) return false;
    hr = nextDevice->Commit();
    if (FAILED(hr)) return false;

    dcompDevice = nextDevice;
    dcompTarget = nextTarget;
    dcompVisual = nextVisual;
    swapChain1.As(&swapChain);
    return swapChain != nullptr;
}

void Win32Window::recreateSwapChain() {
    if (!grContext || !commandQueue) return;

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

    frames.clear();
    swapChain.Reset();
    dcompVisual.Reset();
    dcompTarget.Reset();
    dcompDevice.Reset();
    if (createSwapChain()) {
        resizeBuffers(pixelWidth, pixelHeight);
    }
}

} // namespace SphereUI
