#include <SPHXSvg.hpp>
#include <include/core/SkStream.h>
#include <modules/svg/include/SkSVGDOM.h>
#include <modules/svg/include/SkSVGSVG.h>
#include <modules/svg/include/SkSVGRenderContext.h>
#include <core/ResourceManager.hpp>
#include <fstream>

namespace SphereUI {

// ── Pimpl ─────────────────────────────────────────────────────────────────────

struct SPHXSvg::Impl {
    sk_sp<SkSVGDOM> dom;
};

// ── Internal helpers ──────────────────────────────────────────────────────────

static std::shared_ptr<SPHXSvg::Impl> makeImpl(sk_sp<SkData> data) {
    if (!data || data->isEmpty()) return nullptr;
    SkMemoryStream stream(data->data(), data->size());
    auto dom = SkSVGDOM::MakeFromStream(stream);
    if (!dom) return nullptr;
    auto impl = std::make_shared<SPHXSvg::Impl>();
    impl->dom = std::move(dom);
    return impl;
}

static sk_sp<SkData> readFileData(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return nullptr;
    auto sz = static_cast<size_t>(f.tellg());
    f.seekg(0);
    auto data = SkData::MakeUninitialized(sz);
    if (!f.read(static_cast<char*>(data->writable_data()), sz)) return nullptr;
    return data;
}

// ── Factories ─────────────────────────────────────────────────────────────────

SPHXSvg SPHXSvg::fromFile(const std::string& path) {
    return SPHXSvg(makeImpl(readFileData(path)));
}

SPHXSvg SPHXSvg::fromMemory(const void* data, size_t size) {
    return SPHXSvg(makeImpl(SkData::MakeWithCopy(data, size)));
}

SPHXSvg SPHXSvg::fromString(const std::string& svgText) {
    return SPHXSvg(makeImpl(SkData::MakeWithCopy(svgText.data(), svgText.size())));
}

SPHXSvg SPHXSvg::fromResource(const std::string& resUri) {
    const std::string prefix = "res://";
    std::string key = resUri.substr(0, prefix.size()) == prefix
                    ? resUri.substr(prefix.size()) : resUri;

    auto& rm = ResourceManager::getInstance();
    auto rd  = rm.getResource(key);
    if (!rd.data || rd.size == 0) return SPHXSvg{};

    return SPHXSvg(makeImpl(SkData::MakeWithCopy(rd.data, rd.size)));
}

// ── State ─────────────────────────────────────────────────────────────────────

SkSize SPHXSvg::intrinsicSize() const {
    if (!_impl) return SkSize::MakeEmpty();
    return _impl->dom->containerSize();
}

// ── Rendering helpers ─────────────────────────────────────────────────────────

static void renderScaled(SkSVGDOM* dom, SkCanvas* canvas,
                         float x, float y, float w, float h) {
    SkSize sz = dom->containerSize();
    float sw = sz.width(), sh = sz.height();
    canvas->save();
    canvas->translate(x, y);
    if (sw > 0 && sh > 0)
        canvas->scale(w / sw, h / sh);
    dom->render(canvas);
    canvas->restore();
}

// ── Rendering ─────────────────────────────────────────────────────────────────

void SPHXSvg::render(SkCanvas* canvas, float x, float y) const {
    if (!_impl) return;
    canvas->save();
    canvas->translate(x, y);
    _impl->dom->render(canvas);
    canvas->restore();
}

void SPHXSvg::render(SkCanvas* canvas, const SkRect& dst) const {
    if (!_impl) return;
    renderScaled(_impl->dom.get(), canvas,
                 dst.left(), dst.top(), dst.width(), dst.height());
}

void SPHXSvg::render(SkCanvas* canvas, float x, float y, float w, float h) const {
    if (!_impl) return;
    renderScaled(_impl->dom.get(), canvas, x, y, w, h);
}

void SPHXSvg::render(SkCanvas* canvas, const SkRect& dst, float opacity) const {
    if (!_impl) return;

    // Render into a saveLayer so we can apply opacity
    SkPaint layerPaint;
    layerPaint.setAlphaf(std::max(0.f, std::min(1.f, opacity)));
    canvas->saveLayer(dst, &layerPaint);
    renderScaled(_impl->dom.get(), canvas,
                 dst.left(), dst.top(), dst.width(), dst.height());
    canvas->restore();
}

void SPHXSvg::renderNode(SkCanvas* canvas, const SkRect& dst,
                        const char* nodeId) const {
    if (!_impl || !nodeId) return;

    SkSVGPresentationContext ctx;
    canvas->save();
    canvas->translate(dst.left(), dst.top());

    SkSize sz = _impl->dom->containerSize();
    if (sz.width() > 0 && sz.height() > 0)
        canvas->scale(dst.width() / sz.width(), dst.height() / sz.height());

    _impl->dom->renderNode(canvas, ctx, nodeId);
    canvas->restore();
}

} // namespace SphereUI
