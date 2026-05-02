#include <audio/Streaming.hpp>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace AureliaUI {
namespace Audio {

// ── WAV header structures ─────────────────────────────────────────────────────
namespace {

#pragma pack(push, 1)
struct RiffHeader {
    char     id[4];      // "RIFF"
    uint32_t size;
    char     format[4];  // "WAVE"
};
struct ChunkHeader {
    char     id[4];
    uint32_t size;
};
struct FmtChunk {
    uint16_t audioFormat;    // 1 = PCM, 3 = IEEE float
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};
#pragma pack(pop)

constexpr uint16_t kFmtPCM   = 1;
constexpr uint16_t kFmtFloat = 3;

// Convert a raw byte buffer to interleaved float samples.
// Handles 16-bit integer and 32-bit float WAV.
void convertToFloat(const uint8_t* src, float* dst,
                    size_t frameCount, unsigned int channels,
                    uint16_t audioFormat, uint16_t bitsPerSample)
{
    size_t totalSamples = frameCount * channels;
    if (audioFormat == kFmtFloat && bitsPerSample == 32) {
        std::memcpy(dst, src, totalSamples * sizeof(float));
    } else if (audioFormat == kFmtPCM && bitsPerSample == 16) {
        const int16_t* s = reinterpret_cast<const int16_t*>(src);
        constexpr float inv = 1.f / 32768.f;
        for (size_t i = 0; i < totalSamples; ++i)
            dst[i] = static_cast<float>(s[i]) * inv;
    } else if (audioFormat == kFmtPCM && bitsPerSample == 24) {
        constexpr float inv = 1.f / 8388608.f;
        for (size_t i = 0; i < totalSamples; ++i) {
            int32_t v = (src[0]) | (src[1] << 8) | ((int8_t)src[2] << 16);
            dst[i] = static_cast<float>(v) * inv;
            src += 3;
        }
    } else if (audioFormat == kFmtPCM && bitsPerSample == 8) {
        constexpr float inv = 1.f / 128.f;
        for (size_t i = 0; i < totalSamples; ++i)
            dst[i] = (static_cast<float>(src[i]) - 128.f) * inv;
    }
}

} // anonymous namespace

// ══════════════════════════════════════════════════════════════════════════════
//  WavSource::Impl
// ══════════════════════════════════════════════════════════════════════════════
struct WavSource::Impl {
    std::vector<uint8_t> fileData;   // entire file in memory
    size_t               dataOffset; // byte offset to PCM data in fileData
    size_t               dataSize;   // PCM byte count
    size_t               readPos;    // current read position in bytes
    AudioInfo            info;
    uint16_t             audioFormat;
    uint16_t             bitsPerSample;
    unsigned int         blockAlign;

    bool parse() {
        if (fileData.size() < sizeof(RiffHeader)) return false;
        const uint8_t* p   = fileData.data();
        const uint8_t* end = p + fileData.size();

        const RiffHeader* riff = reinterpret_cast<const RiffHeader*>(p);
        if (std::memcmp(riff->id, "RIFF", 4) != 0) return false;
        if (std::memcmp(riff->format, "WAVE", 4) != 0) return false;
        p += sizeof(RiffHeader);

        bool hasFmt = false, hasData = false;
        FmtChunk fmt{};

        while (p + sizeof(ChunkHeader) <= end) {
            const ChunkHeader* ch = reinterpret_cast<const ChunkHeader*>(p);
            p += sizeof(ChunkHeader);

            if (std::memcmp(ch->id, "fmt ", 4) == 0) {
                if (ch->size < 16) return false;
                std::memcpy(&fmt, p, std::min<uint32_t>(ch->size, sizeof(FmtChunk)));
                hasFmt = true;
            } else if (std::memcmp(ch->id, "data", 4) == 0) {
                dataOffset = static_cast<size_t>(p - fileData.data());
                dataSize   = ch->size;
                hasData    = true;
            }
            p += ((ch->size + 1) & ~1u);  // chunks are word-aligned
        }

        if (!hasFmt || !hasData) return false;

        info.channels      = fmt.numChannels;
        info.sampleRate    = fmt.sampleRate;
        info.bitsPerSample = fmt.bitsPerSample;
        info.format        = AudioFormat::WAV;
        audioFormat        = fmt.audioFormat;
        bitsPerSample      = fmt.bitsPerSample;
        blockAlign         = fmt.blockAlign;

        unsigned int bytesPerSample = bitsPerSample / 8;
        size_t totalFrames = (bytesPerSample > 0 && fmt.numChannels > 0)
            ? dataSize / (bytesPerSample * fmt.numChannels) : 0;
        info.durationSeconds = (fmt.sampleRate > 0)
            ? static_cast<double>(totalFrames) / fmt.sampleRate : 0.0;

        readPos = 0;
        return true;
    }
};

// ── WavSource factory helpers ─────────────────────────────────────────────────
static bool readFile(const std::string& path, std::vector<uint8_t>& out) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return false;
    auto sz = f.tellg(); f.seekg(0);
    out.resize(static_cast<size_t>(sz));
    return static_cast<bool>(f.read(reinterpret_cast<char*>(out.data()), sz));
}

WavSource::WavSource(std::unique_ptr<Impl> impl) : _impl(std::move(impl)) {}
WavSource::~WavSource() = default;

std::unique_ptr<WavSource> WavSource::open(const std::string& path) {
    auto impl = std::make_unique<Impl>();
    if (!readFile(path, impl->fileData) || !impl->parse()) return nullptr;
    return std::unique_ptr<WavSource>(new WavSource(std::move(impl)));
}

std::unique_ptr<WavSource> WavSource::openFromMemory(const void* data, size_t size) {
    auto impl = std::make_unique<Impl>();
    impl->fileData.assign(static_cast<const uint8_t*>(data),
                          static_cast<const uint8_t*>(data) + size);
    if (!impl->parse()) return nullptr;
    return std::unique_ptr<WavSource>(new WavSource(std::move(impl)));
}

AudioInfo WavSource::getInfo()  const { return _impl->info; }
bool      WavSource::isOpen()   const { return _impl != nullptr; }
bool      WavSource::atEnd()    const { return _impl->readPos >= _impl->dataSize; }

double WavSource::position() const {
    if (!_impl || _impl->blockAlign == 0) return 0.0;
    unsigned int frames = static_cast<unsigned int>(_impl->readPos / _impl->blockAlign);
    return (double)frames / _impl->info.sampleRate;
}

bool WavSource::seek(double seconds) {
    if (!_impl || _impl->info.sampleRate == 0) return false;
    size_t frame   = static_cast<size_t>(seconds * _impl->info.sampleRate);
    size_t bytePos = frame * _impl->blockAlign;
    _impl->readPos = std::min(bytePos, _impl->dataSize);
    return true;
}

unsigned int WavSource::read(float* buffer, unsigned int frames) {
    if (!_impl || atEnd()) return 0;

    size_t bytesPerFrame = _impl->blockAlign;
    size_t bytesWanted   = frames * bytesPerFrame;
    size_t bytesAvail    = _impl->dataSize - _impl->readPos;
    size_t bytesRead     = std::min(bytesWanted, bytesAvail);
    unsigned int framesRead = static_cast<unsigned int>(bytesRead / bytesPerFrame);

    if (framesRead == 0) return 0;

    const uint8_t* src = _impl->fileData.data() + _impl->dataOffset + _impl->readPos;
    convertToFloat(src, buffer, framesRead, _impl->info.channels,
                   _impl->audioFormat, _impl->bitsPerSample);
    _impl->readPos += framesRead * bytesPerFrame;
    return framesRead;
}

// ══════════════════════════════════════════════════════════════════════════════
//  BufferSource
// ══════════════════════════════════════════════════════════════════════════════
std::unique_ptr<BufferSource> BufferSource::fromFile(const std::string& path) {
    auto wav = WavSource::open(path);
    if (!wav) return nullptr;

    AudioInfo info = wav->getInfo();
    size_t totalFrames = static_cast<size_t>(info.durationSeconds * info.sampleRate + 0.5);
    std::vector<float> data(totalFrames * info.channels, 0.f);

    unsigned int done = 0;
    while (done < totalFrames) {
        unsigned int got = wav->read(data.data() + done * info.channels,
                                     static_cast<unsigned int>(totalFrames - done));
        if (got == 0) break;
        done += got;
    }
    data.resize(done * info.channels);

    auto src = std::unique_ptr<BufferSource>(new BufferSource());
    src->_data = std::move(data);
    src->_info = info;
    src->_info.format = AudioFormat::PCM_Raw;
    return src;
}

std::unique_ptr<BufferSource> BufferSource::fromRaw(std::vector<float> samples,
                                                      unsigned int channels,
                                                      unsigned int sampleRate) {
    auto src = std::unique_ptr<BufferSource>(new BufferSource());
    src->_data = std::move(samples);
    src->_info.channels      = channels;
    src->_info.sampleRate    = sampleRate;
    src->_info.bitsPerSample = 32;
    src->_info.format        = AudioFormat::PCM_Raw;
    src->_info.durationSeconds = (channels > 0 && sampleRate > 0)
        ? (double)src->_data.size() / channels / sampleRate : 0.0;
    return src;
}

double BufferSource::position() const {
    if (_info.channels == 0 || _info.sampleRate == 0) return 0.0;
    return (double)(_cursor / _info.channels) / _info.sampleRate;
}

bool BufferSource::seek(double seconds) {
    size_t frame = static_cast<size_t>(seconds * _info.sampleRate);
    _cursor = std::min(frame * _info.channels, _data.size());
    return true;
}

unsigned int BufferSource::read(float* buffer, unsigned int frames) {
    if (atEnd()) {
        if (_looping) { _cursor = 0; }
        else          { return 0; }
    }

    size_t samplesWanted = frames * _info.channels;
    size_t samplesAvail  = _data.size() - _cursor;

    if (samplesAvail >= samplesWanted) {
        std::memcpy(buffer, _data.data() + _cursor, samplesWanted * sizeof(float));
        _cursor += samplesWanted;
        return frames;
    }

    // Near end — fill what we have, then loop or stop
    std::memcpy(buffer, _data.data() + _cursor, samplesAvail * sizeof(float));
    unsigned int framesGot = static_cast<unsigned int>(samplesAvail / _info.channels);
    _cursor = _data.size();

    if (_looping) {
        _cursor = 0;
        unsigned int remaining = frames - framesGot;
        framesGot += read(buffer + framesGot * _info.channels, remaining);
    }
    return framesGot;
}

// ══════════════════════════════════════════════════════════════════════════════
//  AudioStreamer
// ══════════════════════════════════════════════════════════════════════════════
AudioStreamer::AudioStreamer(std::unique_ptr<AudioSource> source)
    : _source(std::move(source)) {}

void AudioStreamer::play()  { if (_source) _playing = true;  }
void AudioStreamer::pause() { _playing = false; }
void AudioStreamer::stop()  { _playing = false; if (_source) _source->seek(0.0); }

bool AudioStreamer::seek(double seconds) {
    return _source ? _source->seek(seconds) : false;
}

double AudioStreamer::position() const {
    return _source ? _source->position() : 0.0;
}

void AudioStreamer::fillBuffer(float* outputBuffer,
                               unsigned int frames,
                               unsigned int channels) {
    if (!_playing || !_source) return;

    const AudioInfo& info = _source->getInfo();
    unsigned int srcCh    = info.channels;

    // Temporary mono/native-channel buffer
    std::vector<float> temp(frames * srcCh, 0.f);
    unsigned int framesRead = _source->read(temp.data(), frames);

    if (framesRead == 0) {
        if (_source->atEnd()) {
            _playing = false;
            if (_onComplete) _onComplete();
        }
        return;
    }

    // Mix into outputBuffer with volume, upmix/downmix channels as needed
    for (unsigned int f = 0; f < framesRead; ++f) {
        for (unsigned int c = 0; c < channels; ++c) {
            unsigned int srcIdx = (srcCh > 1) ? (f * srcCh + (c % srcCh)) : (f * srcCh);
            outputBuffer[f * channels + c] += temp[srcIdx] * _volume;
        }
    }

    if (framesRead < frames && _source->atEnd()) {
        _playing = false;
        if (_onComplete) _onComplete();
    }
}

} // namespace Audio
} // namespace AureliaUI
