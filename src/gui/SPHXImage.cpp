#include <SPHXImage.hpp>
#include <include/core/SkStream.h>
#include <include/codec/SkCodec.h>
#include <include/codec/SkPngDecoder.h>
#include <include/codec/SkJpegDecoder.h>
#include <include/codec/SkWebpDecoder.h>
#include <include/codec/SkBmpDecoder.h>
#include <include/codec/SkGifDecoder.h>
#include <core/ResourceManager.hpp>
#include <fstream>

namespace SphereUI {

// ── Internal helpers ──────────────────────────────────────────────────────────

static sk_sp<SkData> readFileData(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return nullptr;
    auto sz = static_cast<size_t>(f.tellg());
    f.seekg(0);
    auto data = SkData::MakeUninitialized(sz);
    if (!f.read(static_cast<char*>(data->writable_data()), sz)) return nullptr;
    return data;
}

sk_sp<SkImage> SPHXImage::decodeFromData(sk_sp<SkData> data) {
    if (!data || data->isEmpty()) return nullptr;

    // Register common decoders
    static bool decodersRegistered = false;
    if (!decodersRegistered) {
        decodersRegistered = true;
        SkCodecs::Register(SkPngDecoder::Decoder());
        SkCodecs::Register(SkJpegDecoder::Decoder());
        SkCodecs::Register(SkWebpDecoder::Decoder());
        SkCodecs::Register(SkBmpDecoder::Decoder());
        SkCodecs::Register(SkGifDecoder::Decoder());
    }

    auto stream = SkMemoryStream::Make(data);
    SkCodec::Result result;
    auto codec = SkCodec::MakeFromStream(std::move(stream), &result);
    if (!codec) return nullptr;

    SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType)
                                       .makeAlphaType(kPremul_SkAlphaType);
    auto bitmap = std::make_unique<SkBitmap>();
    if (!bitmap->tryAllocPixels(info)) return nullptr;

    if (codec->getPixels(info, bitmap->getPixels(), bitmap->rowBytes()) != SkCodec::kSuccess)
        return nullptr;

    bitmap->setImmutable();
    return bitmap->asImage();
}

// ── Factories ─────────────────────────────────────────────────────────────────

SPHXImage SPHXImage::fromFile(const std::string& path) {
    auto data = readFileData(path);
    return SPHXImage(decodeFromData(std::move(data)));
}

SPHXImage SPHXImage::fromMemory(const void* data, size_t size) {
    auto skData = SkData::MakeWithCopy(data, size);
    return SPHXImage(decodeFromData(std::move(skData)));
}

SPHXImage SPHXImage::fromResource(const std::string& resUri) {
    // Strip "res://" prefix
    const std::string prefix = "res://";
    std::string key = resUri.substr(0, prefix.size()) == prefix
                    ? resUri.substr(prefix.size()) : resUri;

    auto& rm = ResourceManager::getInstance();
    auto rd  = rm.getResource(key);
    if (!rd.data || rd.size == 0) return SPHXImage{};

    auto skData = SkData::MakeWithCopy(rd.data, rd.size);
    return SPHXImage(decodeFromData(std::move(skData)));
}

// ── Drawing ───────────────────────────────────────────────────────────────────

void SPHXImage::draw(SkCanvas* canvas, float x, float y,
                    SkSamplingOptions sampling) const {
    if (!_image) return;
    canvas->drawImage(_image, x, y, sampling);
}

void SPHXImage::draw(SkCanvas* canvas, const SkRect& dst,
                    SkSamplingOptions sampling) const {
    if (!_image) return;
    canvas->drawImageRect(_image, dst, sampling);
}

void SPHXImage::draw(SkCanvas* canvas, const SkRect& src, const SkRect& dst,
                    SkSamplingOptions sampling) const {
    if (!_image) return;
    canvas->drawImageRect(_image, src, dst, sampling, nullptr,
                          SkCanvas::kStrict_SrcRectConstraint);
}

void SPHXImage::draw(SkCanvas* canvas, const SkRect& dst, float opacity,
                    SkSamplingOptions sampling) const {
    if (!_image) return;
    SkPaint paint;
    paint.setAlphaf(std::max(0.f, std::min(1.f, opacity)));
    canvas->drawImageRect(_image, dst, sampling, &paint);
}

void SPHXImage::drawFit(SkCanvas* canvas, const SkRect& dst,
                       SkSamplingOptions sampling) const {
    if (!_image) return;
    float iw = static_cast<float>(_image->width());
    float ih = static_cast<float>(_image->height());
    if (iw <= 0 || ih <= 0) return;

    float dw = dst.width(), dh = dst.height();
    float scale = std::min(dw / iw, dh / ih);
    float fw = iw * scale, fh = ih * scale;
    float ox = dst.left() + (dw - fw) * 0.5f;
    float oy = dst.top()  + (dh - fh) * 0.5f;

    canvas->drawImageRect(_image, SkRect::MakeXYWH(ox, oy, fw, fh), sampling);
}

void SPHXImage::drawFill(SkCanvas* canvas, const SkRect& dst,
                        SkSamplingOptions sampling) const {
    if (!_image) return;
    float iw = static_cast<float>(_image->width());
    float ih = static_cast<float>(_image->height());
    if (iw <= 0 || ih <= 0) return;

    float dw = dst.width(), dh = dst.height();
    float scale = std::max(dw / iw, dh / ih);
    float fw = iw * scale, fh = ih * scale;
    float ox = (iw - dw / scale) * 0.5f;
    float oy = (ih - dh / scale) * 0.5f;

    SkRect src = SkRect::MakeXYWH(ox, oy, dw / scale, dh / scale);
    canvas->drawImageRect(_image, src, dst, sampling, nullptr,
                          SkCanvas::kStrict_SrcRectConstraint);
}

} // namespace SphereUI
