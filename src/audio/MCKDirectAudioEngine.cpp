#include <MCKDirectAudioEngine.hpp>
#define RTAUDIO_EXPORT
#include <RtAudio.h>
#include <iostream>

using namespace rt::audio;

namespace MochiUI {
namespace Audio {

class DAUx::Impl {
public:
    RtAudio* dac = nullptr;
    std::function<void(float*, unsigned int)> userCallback;

    Impl(AudioBackend backend) {
        RtAudio::Api api = RtAudio::UNSPECIFIED;
        switch (backend) {
            case AudioBackend::WASAPI: api = RtAudio::WINDOWS_WASAPI; break;
            case AudioBackend::DirectSound: api = RtAudio::WINDOWS_DS; break;
            default: api = RtAudio::UNSPECIFIED; break;
        }
        dac = new RtAudio(api);
    }

    ~Impl() {
        if (dac) {
            if (dac->isStreamOpen()) dac->closeStream();
            delete dac;
        }
    }

    static int rtAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                               double streamTime, RtAudioStreamStatus status, void *userData) {
        Impl* impl = static_cast<Impl*>(userData);
        if (impl && impl->userCallback) {
            impl->userCallback(static_cast<float*>(outputBuffer), nBufferFrames);
        }
        return 0;
    }
};

DAUx::DAUx(AudioBackend backend) : pImpl(new Impl(backend)) {}

DAUx::~DAUx() {
    delete pImpl;
}

std::vector<AudioDevice> DAUx::getDevices() {
    std::vector<AudioDevice> devices;
    std::vector<unsigned int> ids = pImpl->dac->getDeviceIds();

    for (unsigned int id : ids) {
        RtAudio::DeviceInfo info = pImpl->dac->getDeviceInfo(id);
        AudioDevice dev;
        dev.id = id;
        dev.name = info.name;
        dev.inputChannels = info.inputChannels;
        dev.outputChannels = info.outputChannels;
        dev.sampleRates = info.sampleRates;
        dev.preferredSampleRate = info.preferredSampleRate;
        dev.isDefault = info.isDefaultOutput;
        devices.push_back(dev);
    }
    return devices;
}

bool DAUx::openStream(unsigned int deviceId, unsigned int channels, unsigned int sampleRate,
                    unsigned int bufferSize, std::function<void(float*, unsigned int)> callback) {
    pImpl->userCallback = callback;
    RtAudio::StreamParameters parameters;
    parameters.deviceId = deviceId;
    parameters.nChannels = channels;
    parameters.firstChannel = 0;

    try {
        if (pImpl->dac->openStream(&parameters, nullptr, RTAUDIO_FLOAT32, sampleRate, &bufferSize, &Impl::rtAudioCallback, pImpl)) {
            std::cerr << "DAUx Error: " << pImpl->dac->getErrorText() << std::endl;
            return false;
        }
        return true;
    } catch (...) {
        // Fallback for unexpected errors, though RtAudio 6 doesn't throw its own exceptions anymore
        return false;
    }
}

void DAUx::closeStream() {
    if (pImpl->dac->isStreamOpen()) {
        pImpl->dac->closeStream();
    }
}

void DAUx::startStream() {
    if (pImpl->dac->isStreamOpen() && !pImpl->dac->isStreamRunning()) {
        pImpl->dac->startStream();
    }
}

void DAUx::stopStream() {
    if (pImpl->dac->isStreamRunning()) {
        pImpl->dac->stopStream();
    }
}

bool DAUx::isStreamOpen() const {
    return pImpl->dac->isStreamOpen();
}

bool DAUx::isStreamRunning() const {
    return pImpl->dac->isStreamRunning();
}

} // namespace Audio
} // namespace MochiUI
