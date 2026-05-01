#pragma once
#include <AUKFoundation.hpp>
#include <string>
#include <vector>
#include <functional>

#ifdef DAUX_EXPORTS
    #define DAUX_API __declspec(dllexport)
#else
    #define DAUX_API __declspec(dllimport)
#endif

namespace AureliaUI {
namespace Audio {

enum class AudioBackend {
    Default,
    WASAPI,
    DirectSound,
};

struct AudioDevice {
    unsigned int id;
    std::string name;
    unsigned int inputChannels;
    unsigned int outputChannels;
    std::vector<unsigned int> sampleRates;
    unsigned int preferredSampleRate;
    bool isDefault;
};

class DAUX_API DAUx {
public:
    DAUx(AudioBackend backend = AudioBackend::Default);
    ~DAUx();

    std::vector<AudioDevice> getDevices();
    bool openStream(unsigned int deviceId, unsigned int channels, unsigned int sampleRate,
                    unsigned int bufferSize, std::function<void(float*, unsigned int)> callback);
    void closeStream();
    void startStream();
    void stopStream();

    bool isStreamOpen() const;
    bool isStreamRunning() const;

private:
    class Impl;
    Impl* pImpl;
};

} // namespace Audio
} // namespace AureliaUI
