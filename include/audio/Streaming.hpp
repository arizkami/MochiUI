#pragma once
#include <AUKFoundation.hpp>

// AureliaUI audio streaming — pull-model audio sources and a high-level player.
//
// Pattern:
//   auto src      = WavSource::open("sfx/hit.wav");
//   auto streamer = std::make_shared<AudioStreamer>(std::move(src));
//   streamer->setVolume(0.8f);
//   streamer->play();
//   // In your DAUx callback: streamer->fillBuffer(out, frames, channels);

namespace AureliaUI {
namespace Audio {

// ── Audio format tags ─────────────────────────────────────────────────────────
enum class AudioFormat { WAV, MP3, OGG, FLAC, PCM_Raw, Unknown };

// ── Metadata for an audio source ─────────────────────────────────────────────
struct AudioInfo {
    unsigned int channels       = 0;
    unsigned int sampleRate     = 0;
    unsigned int bitsPerSample  = 0;
    double       durationSeconds = 0.0;
    AudioFormat  format          = AudioFormat::Unknown;
};

// ══════════════════════════════════════════════════════════════════════════════
//  AudioSource — abstract pull-model source
// ══════════════════════════════════════════════════════════════════════════════
class AudioSource {
public:
    virtual ~AudioSource() = default;

    virtual AudioInfo    getInfo()   const = 0;
    virtual bool         isOpen()    const = 0;
    virtual bool         atEnd()     const = 0;
    virtual double       position()  const = 0;   // seconds
    virtual bool         seek(double seconds) = 0;

    // Read up to `frames` interleaved float samples.
    // Returns the number of frames actually written (may be < frames at EOF).
    virtual unsigned int read(float* buffer, unsigned int frames) = 0;
};

// ══════════════════════════════════════════════════════════════════════════════
//  WavSource — streaming WAV / PCM file reader
//  Supports 16-bit integer PCM and 32-bit float PCM, mono or multi-channel.
// ══════════════════════════════════════════════════════════════════════════════
class WavSource : public AudioSource {
public:
    ~WavSource() override;

    // Open from a file path
    static std::unique_ptr<WavSource> open(const std::string& path);
    // Open from an in-memory buffer (copies the data)
    static std::unique_ptr<WavSource> openFromMemory(const void* data, size_t size);

    AudioInfo    getInfo()   const override;
    bool         isOpen()    const override;
    bool         atEnd()     const override;
    double       position()  const override;
    bool         seek(double seconds) override;
    unsigned int read(float* buffer, unsigned int frames) override;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
    explicit WavSource(std::unique_ptr<Impl> impl);
};

// ══════════════════════════════════════════════════════════════════════════════
//  BufferSource — entire audio file pre-loaded into RAM
//  Ideal for short clips and sound effects; instant seeking with zero latency.
// ══════════════════════════════════════════════════════════════════════════════
class BufferSource : public AudioSource {
public:
    // Load and decode a WAV file fully into memory
    static std::unique_ptr<BufferSource> fromFile(const std::string& path);

    // Wrap a pre-existing float sample array (interleaved)
    static std::unique_ptr<BufferSource> fromRaw(std::vector<float> samples,
                                                  unsigned int channels,
                                                  unsigned int sampleRate);

    AudioInfo    getInfo()   const override { return _info; }
    bool         isOpen()    const override { return !_data.empty(); }
    bool         atEnd()     const override { return _cursor >= _data.size(); }
    double       position()  const override;
    bool         seek(double seconds) override;
    unsigned int read(float* buffer, unsigned int frames) override;

    void setLooping(bool loop) { _looping = loop; }
    bool isLooping()     const { return _looping; }

    // Total sample count
    size_t sampleCount() const { return _data.size(); }

private:
    std::vector<float> _data;
    AudioInfo          _info;
    size_t             _cursor  = 0;
    bool               _looping = false;

    BufferSource() = default;
};

// ══════════════════════════════════════════════════════════════════════════════
//  AudioStreamer — high-level player that wraps any AudioSource
//  Provides play / pause / stop / seek with per-stream volume.
//  Thread-safe: fillBuffer() may be called from the DAUx audio thread.
// ══════════════════════════════════════════════════════════════════════════════
class AudioStreamer {
public:
    explicit AudioStreamer(std::unique_ptr<AudioSource> source);

    // ── Playback control ──────────────────────────────────────────────────────
    void play();
    void pause();
    void stop();    // pause + seek to start
    bool isPlaying() const { return _playing; }

    // ── Volume & gain ─────────────────────────────────────────────────────────
    void  setVolume(float v)  { _volume = v < 0.f ? 0.f : v > 1.f ? 1.f : v; }
    float getVolume()   const { return _volume; }

    // ── Seeking ───────────────────────────────────────────────────────────────
    bool   seek(double seconds);
    double position()  const;
    double duration()  const { return _source ? _source->getInfo().durationSeconds : 0.0; }

    // ── Metadata ──────────────────────────────────────────────────────────────
    AudioInfo info() const { return _source ? _source->getInfo() : AudioInfo{}; }

    // ── Callbacks ─────────────────────────────────────────────────────────────
    void setOnComplete(std::function<void()> cb) { _onComplete = std::move(cb); }

    // ── Engine integration ────────────────────────────────────────────────────
    // Call from inside your DAUx audio callback. Mixes this streamer's output
    // into outputBuffer (interleaved float, already zeroed or accumulated).
    void fillBuffer(float* outputBuffer, unsigned int frames, unsigned int channels);

private:
    std::unique_ptr<AudioSource> _source;
    bool                         _playing    = false;
    float                        _volume     = 1.f;
    std::function<void()>        _onComplete;
};

} // namespace Audio
} // namespace AureliaUI
