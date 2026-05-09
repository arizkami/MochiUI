#pragma once
#include <SPHXFoundation.hpp>

#ifdef DAUX_EXPORTS
    #define DAUX_API __declspec(dllexport)
#else
    #define DAUX_API __declspec(dllimport)
#endif

namespace SphereUI {
namespace Audio {

// ── Backend selection ─────────────────────────────────────────────────────────
enum class AudioBackend {
    Default,
    WASAPI,
    DirectSound,
};

// ── Device descriptor ─────────────────────────────────────────────────────────
struct AudioDevice {
    unsigned int              id;
    std::string               name;
    unsigned int              inputChannels;
    unsigned int              outputChannels;
    std::vector<unsigned int> sampleRates;
    unsigned int              preferredSampleRate;
    bool                      isDefaultInput;
    bool                      isDefaultOutput;
};

// ── Audio effect base ─────────────────────────────────────────────────────────
// Derive from AudioEffect to build custom DSP processors. Each effect receives
// an interleaved float buffer (channels × frames) in place.
class DAUX_API AudioEffect {
public:
    virtual ~AudioEffect() = default;
    virtual void process(float* buffer, unsigned int frames,
                         unsigned int channels, unsigned int sampleRate) = 0;
    bool enabled = true;
};

// ── Built-in effects ──────────────────────────────────────────────────────────
// Simple linear gain. gain = 1.0 → unity, 2.0 → +6 dB, 0.0 → silence.
class DAUX_API GainEffect : public AudioEffect {
public:
    float gain = 1.f;
    void process(float* buffer, unsigned int frames,
                 unsigned int channels, unsigned int sampleRate) override;
};

// One-pole low-pass / high-pass filter for tone shaping.
class DAUX_API LowPassEffect : public AudioEffect {
public:
    float cutoffHz = 8000.f;
    void process(float* buffer, unsigned int frames,
                 unsigned int channels, unsigned int sampleRate) override;
private:
    float _z = 0.f;
};

class DAUX_API HighPassEffect : public AudioEffect {
public:
    float cutoffHz = 80.f;
    void process(float* buffer, unsigned int frames,
                 unsigned int channels, unsigned int sampleRate) override;
private:
    float _z = 0.f;
};

// Hard-knee peak limiter — prevents clipping above threshold.
class DAUX_API LimiterEffect : public AudioEffect {
public:
    float thresholdDb = -0.3f;
    void process(float* buffer, unsigned int frames,
                 unsigned int channels, unsigned int sampleRate) override;
};

// ── Mixer channel ─────────────────────────────────────────────────────────────
// Each mixer channel owns a pull-callback audio source and an effect chain.
struct MixerChannel {
    std::string  name;
    float        volume = 1.f;   // 0.0 – 1.0
    float        pan    = 0.f;   // -1.0 (L) to +1.0 (R)
    bool         muted  = false;
    bool         solo   = false;
    std::vector<std::shared_ptr<AudioEffect>> effects;
    // Called each block to fill `frames` × `channels` interleaved samples
    std::function<void(float*, unsigned int, unsigned int)> source;
};

// ── Main audio engine ─────────────────────────────────────────────────────────
class DAUX_API DAUx {
public:
    explicit DAUx(AudioBackend backend = AudioBackend::Default);
    ~DAUx();

    // ── Device enumeration ────────────────────────────────────────────────────
    std::vector<AudioDevice> getDevices();
    AudioDevice              getDefaultOutputDevice();
    AudioDevice              getDefaultInputDevice();

    // ── Stream lifecycle ──────────────────────────────────────────────────────
    // Low-level: supply your own interleaved-float callback
    bool openStream(unsigned int deviceId,
                    unsigned int channels,
                    unsigned int sampleRate,
                    unsigned int bufferSize,
                    std::function<void(float*, unsigned int, unsigned int)> callback);

    // Convenience: open the system default output device
    bool openDefaultStream(unsigned int channels,
                           unsigned int sampleRate,
                           unsigned int bufferSize,
                           std::function<void(float*, unsigned int, unsigned int)> callback);

    void closeStream();
    void startStream();
    void stopStream();
    bool isStreamOpen()    const;
    bool isStreamRunning() const;

    // ── Stream properties ─────────────────────────────────────────────────────
    unsigned int getSampleRate()   const;
    unsigned int getChannelCount() const;
    unsigned int getBufferSize()   const;
    double       getStreamLatency() const;  // seconds

    // ── Master volume & mute ──────────────────────────────────────────────────
    void  setMasterVolume(float vol);   // 0.0 – 1.0
    float getMasterVolume() const;
    void  setMasterMute(bool mute);
    bool  getMasterMute()  const;

    // ── Master effects chain ──────────────────────────────────────────────────
    void addMasterEffect(std::shared_ptr<AudioEffect> effect);
    void removeMasterEffect(std::shared_ptr<AudioEffect> effect);
    void clearMasterEffects();

    // ── Mixer channels ────────────────────────────────────────────────────────
    int  addChannel(MixerChannel channel);       // returns channel id
    void removeChannel(int channelId);
    void setChannelVolume(int channelId, float vol);
    void setChannelPan(int channelId, float pan);
    void setChannelMute(int channelId, bool mute);
    void setChannelSolo(int channelId, bool solo);
    MixerChannel* getChannel(int channelId);    // nullptr if not found

    // ── Error handling ────────────────────────────────────────────────────────
    void        setErrorCallback(std::function<void(const std::string&)> cb);
    std::string getLastError() const;

private:
    class Impl;
    Impl* pImpl;
};

// ── MIDI ──────────────────────────────────────────────────────────────────────

enum class MidiEventType {
    NoteOff, NoteOn, PolyPressure,
    ControlChange, ProgramChange,
    ChannelPressure, PitchBend, SysEx,
};

struct MidiEvent {
    MidiEventType type;
    uint8_t  channel;     // 0-15
    uint8_t  note;        // NoteOn/Off/PolyPressure: MIDI note 0-127
    uint8_t  velocity;    // NoteOn/Off: velocity 0-127
    uint8_t  controller;  // ControlChange: controller number
    uint8_t  value;       // ControlChange/ProgramChange: 0-127
    int16_t  pitchBend;   // PitchBend: -8192 … +8191
    std::vector<uint8_t> sysExData;
    double   timestamp;   // seconds since port was opened
};

using MidiCallback = std::function<void(const MidiEvent&)>;

struct MidiPort {
    unsigned int id;
    std::string  name;
    bool         isVirtual = false;
};

// MIDI input / output manager (singleton; wraps RtMidi).
class DAUX_API MidiSystem {
public:
    static MidiSystem& getInstance();

    // ── Port enumeration ──────────────────────────────────────────────────────
    std::vector<MidiPort> getInputPorts();
    std::vector<MidiPort> getOutputPorts();

    // ── Input ─────────────────────────────────────────────────────────────────
    bool openInputPort(unsigned int portId, MidiCallback callback);
    void closeInputPort(unsigned int portId);
    bool isInputPortOpen(unsigned int portId) const;

    // ── Output ────────────────────────────────────────────────────────────────
    bool openOutputPort(unsigned int portId);
    void closeOutputPort(unsigned int portId);
    bool isOutputPortOpen(unsigned int portId) const;

    void sendNoteOn(unsigned int portId, uint8_t channel, uint8_t note, uint8_t velocity);
    void sendNoteOff(unsigned int portId, uint8_t channel, uint8_t note, uint8_t velocity = 0);
    void sendControlChange(unsigned int portId, uint8_t channel, uint8_t controller, uint8_t value);
    void sendPitchBend(unsigned int portId, uint8_t channel, int16_t bend);
    void sendRawMessage(unsigned int portId, const std::vector<uint8_t>& message);

    void setErrorCallback(std::function<void(const std::string&)> cb);

private:
    MidiSystem();
    ~MidiSystem();
    class Impl;
    Impl* pImpl;
};

// ── FFT Analysis ──────────────────────────────────────────────────────────────
// Thread-safe: pushSamples is called from the audio thread; getters are safe
// to call from any thread (they return a snapshot copy).
class DAUX_API FFTAnalyzer {
public:
    explicit FFTAnalyzer(unsigned int fftSize = 2048);
    ~FFTAnalyzer();

    // Push interleaved float PCM samples (channels summed to mono internally).
    void pushSamples(const float* buffer, unsigned int frames, unsigned int channels);

    // Latest magnitude spectrum: fftSize/2+1 bins, magnitude in dBFS.
    // Bin 0 = DC, last bin = Nyquist.
    std::vector<float> getMagnitudeSpectrum() const;

    // Peak magnitude in [lowHz, highHz] band, in dBFS.
    float getPeakMagnitude(float lowHz, float highHz, float sampleRate) const;

    // Smoothed RMS of the latest analysis block, in dBFS.
    float getRMSLevel() const;

    unsigned int getFFTSize() const;
    unsigned int getBinCount() const;  // fftSize / 2 + 1

    // Smoothing coefficient applied per-update (0 = no smoothing, 1 = frozen).
    float smoothing = 0.8f;

private:
    class Impl;
    Impl* pImpl;
};

} // namespace Audio
} // namespace SphereUI
