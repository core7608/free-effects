#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace FreeEffect {

struct WaveformData {
    std::vector<float> samples;
    int sampleRate = 44100;
    int channelCount = 1;
};

class WaveformGenerator {
public:
    WaveformGenerator();
    ~WaveformGenerator();
    
    WaveformData generateFromAudio(const std::string& filePath, int targetSamples = 1000);
    WaveformData generateFromSamples(const float* samples, int totalSamples, 
                                      int sourceSampleRate, int channels,
                                      int targetSamples = 1000);
    
    static std::vector<float> downsample(const float* input, int inputLength, int outputLength);

private:
};

} // namespace FreeEffect
