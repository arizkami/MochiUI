#include <AUKDirectAudioEngine.hpp>
#define RTAUDIO_EXPORT
#include <RtAudio.h>
#include <cmath>
#include <algorithm>

namespace AureliaUI {
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

} // namespace Audio
} // namespace AureliaUI
