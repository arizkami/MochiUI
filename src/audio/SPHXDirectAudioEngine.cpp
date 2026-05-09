#include <SPHXDirectAudioEngine.hpp>
#define RTAUDIO_EXPORT
#include <RtAudio.h>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <numeric>

namespace SphereUI {
namespace Audio {

// ── GainEffect ────────────────────────────────────────────────────────────────
void GainEffect::process(float* buf, unsigned int frames,
                         unsigned int channels, unsigned int) {
    if (!enabled) return;
    size_t n = frames * channels;
    for (size_t i = 0; i < n; ++i) buf[i] *= gain;
}

// ── LowPassEffect (one-pole IIR, per-channel state shared for simplicity) ─────
void LowPassEffect::process(float* buf, unsigned int frames,
                            unsigned int channels, unsigned int sampleRate) {
    if (!enabled) return;
    float rc  = 1.f / (2.f * 3.14159265f * cutoffHz);
    float dt  = 1.f / static_cast<float>(sampleRate);
    float a   = dt / (rc + dt);
    for (unsigned int f = 0; f < frames; ++f)
        for (unsigned int c = 0; c < channels; ++c) {
            float& s = buf[f * channels + c];
            _z = _z + a * (s - _z);
            s  = _z;
        }
}

// ── HighPassEffect ────────────────────────────────────────────────────────────
void HighPassEffect::process(float* buf, unsigned int frames,
                             unsigned int channels, unsigned int sampleRate) {
    if (!enabled) return;
    float rc  = 1.f / (2.f * 3.14159265f * cutoffHz);
    float dt  = 1.f / static_cast<float>(sampleRate);
    float a   = rc / (rc + dt);
    float prev = 0.f;
    for (unsigned int f = 0; f < frames; ++f)
        for (unsigned int c = 0; c < channels; ++c) {
            float& s  = buf[f * channels + c];
            float  hp = a * (_z + s - prev);
            prev  = s;
            _z    = hp;
            s     = hp;
        }
}

// ── LimiterEffect ─────────────────────────────────────────────────────────────
void LimiterEffect::process(float* buf, unsigned int frames,
                            unsigned int channels, unsigned int) {
    if (!enabled) return;
    float thresh = std::pow(10.f, thresholdDb / 20.f);
    size_t n     = frames * channels;
    for (size_t i = 0; i < n; ++i)
        if (buf[i] >  thresh) buf[i] =  thresh;
        else if (buf[i] < -thresh) buf[i] = -thresh;
}

// ══════════════════════════════════════════════════════════════════════════════
//  DAUx::Impl
// ══════════════════════════════════════════════════════════════════════════════
class DAUx::Impl {
public:
    RtAudio* dac = nullptr;

    std::function<void(float*, unsigned int, unsigned int)> userCallback;
    std::function<void(const std::string&)>                 errorCallback;

    float        masterVolume  = 1.f;
    bool         masterMuted   = false;
    std::string  lastError;

    unsigned int streamChannels  = 0;
    unsigned int streamRate      = 0;
    unsigned int streamBufSize   = 0;

    std::vector<std::shared_ptr<AudioEffect>> masterEffects;

    // Mixer channels
    struct Channel {
        int          id;
        MixerChannel data;
        std::vector<float> mixBuf;
    };
    std::vector<Channel> channels;
    int nextChannelId = 0;

    explicit Impl(AudioBackend backend) {
        RtAudio::Api api = RtAudio::UNSPECIFIED;
        switch (backend) {
            case AudioBackend::WASAPI:       api = RtAudio::WINDOWS_WASAPI; break;
            case AudioBackend::DirectSound:  api = RtAudio::WINDOWS_DS;     break;
            default:                         api = RtAudio::UNSPECIFIED;     break;
        }
        dac = new RtAudio(api);
    }

    ~Impl() {
        if (dac) {
            if (dac->isStreamOpen()) dac->closeStream();
            delete dac;
        }
    }

    static int rtCallback(void* outBuf, void*, unsigned int nFrames,
                          double, RtAudioStreamStatus, void* ud) {
        auto* impl = static_cast<Impl*>(ud);
        float* out = static_cast<float*>(outBuf);
        unsigned int ch = impl->streamChannels;
        size_t n = nFrames * ch;

        // Zero the output first
        std::fill(out, out + n, 0.f);

        // Mix channels
        for (auto& chan : impl->channels) {
            if (chan.data.muted) continue;
            if (!chan.data.source) continue;
            bool anySolo = false;
            for (auto& c2 : impl->channels) if (c2.data.solo) { anySolo = true; break; }
            if (anySolo && !chan.data.solo) continue;

            chan.mixBuf.assign(n, 0.f);
            chan.data.source(chan.mixBuf.data(), nFrames, ch);

            // Per-channel effects
            for (auto& fx : chan.data.effects)
                if (fx && fx->enabled)
                    fx->process(chan.mixBuf.data(), nFrames, ch, impl->streamRate);

            // Volume + pan
            float vol = chan.data.volume;
            float pan = std::max(-1.f, std::min(1.f, chan.data.pan));
            for (unsigned int f = 0; f < nFrames; ++f) {
                for (unsigned int c = 0; c < ch; ++c) {
                    float panGain = (ch == 2)
                        ? (c == 0 ? (1.f - pan) * 0.5f + 0.5f : (1.f + pan) * 0.5f)
                        : 1.f;
                    out[f * ch + c] += chan.mixBuf[f * ch + c] * vol * panGain;
                }
            }
        }

        // Invoke user callback (may also accumulate into out)
        if (impl->userCallback)
            impl->userCallback(out, nFrames, ch);

        // Master effects
        for (auto& fx : impl->masterEffects)
            if (fx && fx->enabled)
                fx->process(out, nFrames, ch, impl->streamRate);

        // Master volume / mute
        float mv = impl->masterMuted ? 0.f : impl->masterVolume;
        if (mv != 1.f)
            for (size_t i = 0; i < n; ++i) out[i] *= mv;

        return 0;
    }

    void reportError(const std::string& msg) {
        lastError = msg;
        if (errorCallback) errorCallback(msg);
    }

    AudioDevice fillDevice(unsigned int id) {
        RtAudio::DeviceInfo info = dac->getDeviceInfo(id);
        AudioDevice d;
        d.id                  = id;
        d.name                = info.name;
        d.inputChannels       = info.inputChannels;
        d.outputChannels      = info.outputChannels;
        d.sampleRates         = info.sampleRates;
        d.preferredSampleRate = info.preferredSampleRate;
        d.isDefaultOutput     = info.isDefaultOutput;
        d.isDefaultInput      = info.isDefaultInput;
        return d;
    }
};

// ══════════════════════════════════════════════════════════════════════════════
//  DAUx public API
// ══════════════════════════════════════════════════════════════════════════════
DAUx::DAUx(AudioBackend backend) : pImpl(new Impl(backend)) {}
DAUx::~DAUx() { delete pImpl; }

// ── Devices ───────────────────────────────────────────────────────────────────
std::vector<AudioDevice> DAUx::getDevices() {
    std::vector<AudioDevice> out;
    for (unsigned int id : pImpl->dac->getDeviceIds())
        out.push_back(pImpl->fillDevice(id));
    return out;
}

AudioDevice DAUx::getDefaultOutputDevice() {
    return pImpl->fillDevice(pImpl->dac->getDefaultOutputDevice());
}

AudioDevice DAUx::getDefaultInputDevice() {
    return pImpl->fillDevice(pImpl->dac->getDefaultInputDevice());
}

// ── Stream ────────────────────────────────────────────────────────────────────
bool DAUx::openStream(unsigned int deviceId, unsigned int channels,
                      unsigned int sampleRate, unsigned int bufferSize,
                      std::function<void(float*, unsigned int, unsigned int)> cb) {
    pImpl->userCallback    = std::move(cb);
    pImpl->streamChannels  = channels;
    pImpl->streamRate      = sampleRate;
    pImpl->streamBufSize   = bufferSize;

    RtAudio::StreamParameters params;
    params.deviceId    = deviceId;
    params.nChannels   = channels;
    params.firstChannel = 0;

    if (pImpl->dac->openStream(&params, nullptr, RTAUDIO_FLOAT32,
                               sampleRate, &bufferSize,
                               &Impl::rtCallback, pImpl)) {
        pImpl->reportError(pImpl->dac->getErrorText());
        return false;
    }
    return true;
}

bool DAUx::openDefaultStream(unsigned int channels, unsigned int sampleRate,
                              unsigned int bufferSize,
                              std::function<void(float*, unsigned int, unsigned int)> cb) {
    return openStream(pImpl->dac->getDefaultOutputDevice(),
                      channels, sampleRate, bufferSize, std::move(cb));
}

void DAUx::closeStream() {
    if (pImpl->dac->isStreamOpen()) pImpl->dac->closeStream();
}
void DAUx::startStream() {
    if (pImpl->dac->isStreamOpen() && !pImpl->dac->isStreamRunning())
        pImpl->dac->startStream();
}
void DAUx::stopStream() {
    if (pImpl->dac->isStreamRunning()) pImpl->dac->stopStream();
}
bool DAUx::isStreamOpen()    const { return pImpl->dac->isStreamOpen();    }
bool DAUx::isStreamRunning() const { return pImpl->dac->isStreamRunning(); }

// ── Stream info ───────────────────────────────────────────────────────────────
unsigned int DAUx::getSampleRate()    const { return pImpl->streamRate;      }
unsigned int DAUx::getChannelCount()  const { return pImpl->streamChannels;  }
unsigned int DAUx::getBufferSize()    const { return pImpl->streamBufSize;   }
double       DAUx::getStreamLatency() const {
    return pImpl->dac->isStreamOpen() ? pImpl->dac->getStreamLatency() : 0.0;
}

// ── Master volume / mute ──────────────────────────────────────────────────────
void  DAUx::setMasterVolume(float v)  { pImpl->masterVolume = std::max(0.f, std::min(1.f, v)); }
float DAUx::getMasterVolume()  const  { return pImpl->masterVolume; }
void  DAUx::setMasterMute(bool m)     { pImpl->masterMuted = m; }
bool  DAUx::getMasterMute()    const  { return pImpl->masterMuted; }

// ── Master effects ─────────────────────────────────────────────────────────
void DAUx::addMasterEffect(std::shared_ptr<AudioEffect> fx) {
    pImpl->masterEffects.push_back(std::move(fx));
}
void DAUx::removeMasterEffect(std::shared_ptr<AudioEffect> fx) {
    auto& v = pImpl->masterEffects;
    v.erase(std::remove(v.begin(), v.end(), fx), v.end());
}
void DAUx::clearMasterEffects() { pImpl->masterEffects.clear(); }

// ── Mixer channels ────────────────────────────────────────────────────────────
int DAUx::addChannel(MixerChannel channel) {
    int id = pImpl->nextChannelId++;
    pImpl->channels.push_back({ id, std::move(channel), {} });
    return id;
}
void DAUx::removeChannel(int id) {
    auto& v = pImpl->channels;
    v.erase(std::remove_if(v.begin(), v.end(),
        [id](const Impl::Channel& c){ return c.id == id; }), v.end());
}

MixerChannel* DAUx::getChannel(int id) {
    for (auto& c : pImpl->channels)
        if (c.id == id) return &c.data;
    return nullptr;
}

void DAUx::setChannelVolume(int id, float v) {
    if (auto* ch = getChannel(id)) ch->volume = std::max(0.f, std::min(1.f, v));
}
void DAUx::setChannelPan(int id, float p) {
    if (auto* ch = getChannel(id)) ch->pan = std::max(-1.f, std::min(1.f, p));
}
void DAUx::setChannelMute(int id, bool m) {
    if (auto* ch = getChannel(id)) ch->muted = m;
}
void DAUx::setChannelSolo(int id, bool s) {
    if (auto* ch = getChannel(id)) ch->solo = s;
}

// ── Error handling ────────────────────────────────────────────────────────────
void DAUx::setErrorCallback(std::function<void(const std::string&)> cb) {
    pImpl->errorCallback = std::move(cb);
}
std::string DAUx::getLastError() const { return pImpl->lastError; }

// ══════════════════════════════════════════════════════════════════════════════
//  FFTAnalyzer
// ══════════════════════════════════════════════════════════════════════════════
class FFTAnalyzer::Impl {
public:
    const unsigned int fftSize;
    const unsigned int binCount;

    std::vector<float> ring;          // ring buffer (mono, fftSize samples)
    size_t             writePos = 0;
    bool               ringFull = false;

    mutable std::mutex            mtx;
    std::vector<float>            spectrum;  // magnitude in dBFS, binCount entries
    std::atomic<float>            rmsDb{-96.f};
    float                         smoothing = 0.8f;

    explicit Impl(unsigned int sz)
        : fftSize(std::max(8u, sz))
        , binCount(fftSize / 2 + 1)
        , ring(fftSize, 0.f)
        , spectrum(binCount, -96.f)
    {}

    // Simple DFT — accurate but O(N²); suitable for fftSize <= 2048 at low call rate.
    void computeSpectrum() {
        std::vector<float> src(ring.size());
        // Un-rotate ring buffer into a contiguous array
        size_t half = ringFull ? fftSize : writePos;
        for (size_t i = 0; i < half; ++i)
            src[i] = ring[(writePos - half + i + fftSize) % fftSize];

        // RMS
        float sumSq = 0.f;
        for (float s : src) sumSq += s * s;
        float rms = std::sqrt(sumSq / static_cast<float>(src.size() ? src.size() : 1));
        rmsDb.store((rms > 1e-10f) ? 20.f * std::log10(rms) : -96.f,
                    std::memory_order_relaxed);

        const float pi2 = 2.f * 3.14159265358979f;
        const float norm = 1.f / static_cast<float>(fftSize);
        for (unsigned int k = 0; k < binCount; ++k) {
            float re = 0.f, im = 0.f;
            for (unsigned int n = 0; n < fftSize; ++n) {
                float ang = pi2 * k * n * norm;
                float s   = (n < src.size()) ? src[n] : 0.f;
                re += s * std::cos(ang);
                im += s * std::sin(ang);
            }
            float mag = std::sqrt(re * re + im * im) * norm;
            float db  = (mag > 1e-10f) ? 20.f * std::log10(mag) : -96.f;
            // Exponential smoothing towards new value
            float prev = spectrum[k];
            spectrum[k] = smoothing * prev + (1.f - smoothing) * db;
        }
    }
};

FFTAnalyzer::FFTAnalyzer(unsigned int fftSize) : pImpl(new Impl(fftSize)) {}
FFTAnalyzer::~FFTAnalyzer() { delete pImpl; }

void FFTAnalyzer::pushSamples(const float* buffer, unsigned int frames, unsigned int channels) {
    if (!buffer || frames == 0 || channels == 0) return;
    for (unsigned int f = 0; f < frames; ++f) {
        // Downmix to mono
        float mono = 0.f;
        for (unsigned int c = 0; c < channels; ++c)
            mono += buffer[f * channels + c];
        mono /= static_cast<float>(channels);

        pImpl->ring[pImpl->writePos] = mono;
        pImpl->writePos = (pImpl->writePos + 1) % pImpl->fftSize;
        if (pImpl->writePos == 0) pImpl->ringFull = true;

        // Trigger analysis once per full window
        if (pImpl->writePos == 0 || (!pImpl->ringFull && pImpl->writePos == frames - 1)) {
            std::lock_guard<std::mutex> lk(pImpl->mtx);
            pImpl->smoothing = smoothing;
            pImpl->computeSpectrum();
        }
    }
}

std::vector<float> FFTAnalyzer::getMagnitudeSpectrum() const {
    std::lock_guard<std::mutex> lk(pImpl->mtx);
    return pImpl->spectrum;
}

float FFTAnalyzer::getPeakMagnitude(float lowHz, float highHz, float sampleRate) const {
    std::lock_guard<std::mutex> lk(pImpl->mtx);
    if (sampleRate <= 0.f) return -96.f;
    float nyquist = sampleRate * 0.5f;
    float hzPerBin = nyquist / static_cast<float>(pImpl->binCount - 1);
    unsigned int lo = static_cast<unsigned int>(std::max(0.f, lowHz  / hzPerBin));
    unsigned int hi = static_cast<unsigned int>(std::min(static_cast<float>(pImpl->binCount - 1), highHz / hzPerBin));
    float peak = -96.f;
    for (unsigned int k = lo; k <= hi; ++k)
        if (pImpl->spectrum[k] > peak) peak = pImpl->spectrum[k];
    return peak;
}

float FFTAnalyzer::getRMSLevel() const {
    return pImpl->rmsDb.load(std::memory_order_relaxed);
}

unsigned int FFTAnalyzer::getFFTSize()  const { return pImpl->fftSize;   }
unsigned int FFTAnalyzer::getBinCount() const { return pImpl->binCount;  }

// ══════════════════════════════════════════════════════════════════════════════
//  MidiSystem  (stub — wire up RtMidi when rtmidi is in the build)
// ══════════════════════════════════════════════════════════════════════════════
class MidiSystem::Impl {
public:
    std::function<void(const std::string&)> errorCb;
};

MidiSystem::MidiSystem()  : pImpl(new Impl()) {}
MidiSystem::~MidiSystem() { delete pImpl; }

MidiSystem& MidiSystem::getInstance() {
    static MidiSystem instance;
    return instance;
}

std::vector<MidiPort> MidiSystem::getInputPorts()  { return {}; }
std::vector<MidiPort> MidiSystem::getOutputPorts() { return {}; }

bool MidiSystem::openInputPort(unsigned int, MidiCallback)  { return false; }
void MidiSystem::closeInputPort(unsigned int)  {}
bool MidiSystem::isInputPortOpen(unsigned int) const { return false; }

bool MidiSystem::openOutputPort(unsigned int)  { return false; }
void MidiSystem::closeOutputPort(unsigned int) {}
bool MidiSystem::isOutputPortOpen(unsigned int) const { return false; }

void MidiSystem::sendNoteOn(unsigned int, uint8_t, uint8_t, uint8_t) {}
void MidiSystem::sendNoteOff(unsigned int, uint8_t, uint8_t, uint8_t) {}
void MidiSystem::sendControlChange(unsigned int, uint8_t, uint8_t, uint8_t) {}
void MidiSystem::sendPitchBend(unsigned int, uint8_t, int16_t) {}
void MidiSystem::sendRawMessage(unsigned int, const std::vector<uint8_t>&) {}
void MidiSystem::setErrorCallback(std::function<void(const std::string&)> cb) {
    pImpl->errorCb = std::move(cb);
}

} // namespace Audio
} // namespace SphereUI
