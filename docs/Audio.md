# Audio Streaming

AureliaUI includes a pull-model audio streaming system for handling sound effects and background music.

## Core Concepts

### AudioSource

An abstract base class for any audio data source (files, memory buffers, etc.).

- `getInfo()`: Returns metadata (channels, sample rate, duration).
- `read(buffer, frames)`: Reads interleaved float samples.

### WavSource

A streaming reader for WAV files. Efficient for long audio files as it reads from disk on demand.

```cpp
auto src = WavSource::open("music.wav");
```

### BufferSource

Pre-loads the entire audio file into memory. Ideal for short, low-latency sound effects.

```cpp
auto sfx = BufferSource::fromFile("click.wav");
sfx->setLooping(true);
```

### AudioStreamer

A high-level player that wraps an `AudioSource` and provides playback controls.

```cpp
auto streamer = std::make_shared<AudioStreamer>(std::move(src));
streamer->setVolume(0.8f);
streamer->play();
```

## Integration

To use audio, you typically call `fillBuffer` inside your engine's audio callback.

```cpp
void OnAudioCallback(float* buffer, unsigned int frames, unsigned int channels) {
    streamer->fillBuffer(buffer, frames, channels);
}
```
