#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace FreeEffect {

struct AudioFormat {
    int sampleRate = 44100;
    int channels = 2;
    int bitsPerSample = 16;
};

class AudioDecoder {
public:
    AudioDecoder();
    ~AudioDecoder();
    
    bool open(const std::string& filePath);
    void close();
    bool isOpen() const { return m_isOpen; }
    
    const AudioFormat& getFormat() const { return m_format; }
    double getDuration() const { return m_duration; }
    
    // Decode audio samples. Returns number of samples decoded per channel.
    // Output buffer is interleaved (L, R, L, R, ...)
    int decode(float* outputBuffer, int maxSamples);
    
    bool seek(double timeInSeconds);

private:
    bool m_isOpen = false;
    AudioFormat m_format;
    double m_duration = 0.0;
    
#ifdef HAS_FFMPEG
    struct AVContext;
    AVContext* m_context = nullptr;
#endif
};

} // namespace FreeEffect
