#include "waveform_generator.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

WaveformGenerator::WaveformGenerator() {
}

WaveformGenerator::~WaveformGenerator() {
}

WaveformData WaveformGenerator::generateFromAudio(const std::string& filePath, int targetSamples) {
    WaveformData result;
    result.sampleRate = 44100;
    result.channelCount = 1;
    result.samples.resize(targetSamples, 0.0f);
    return result;
}

WaveformData WaveformGenerator::generateFromSamples(const float* samples, int totalSamples,
                                                     int sourceSampleRate, int channels,
                                                     int targetSamples) {
    WaveformData result;
    result.sampleRate = sourceSampleRate;
    result.channelCount = 1;
    
    if (!samples || totalSamples <= 0 || targetSamples <= 0) {
        result.samples.resize(targetSamples, 0.0f);
        return result;
    }
    
    // Convert interleaved to mono if needed
    std::vector<float> mono;
    mono.reserve(totalSamples / channels);
    for (int i = 0; i < totalSamples; i += channels) {
        float sum = 0.0f;
        for (int ch = 0; ch < channels && (i + ch) < totalSamples; ++ch) {
            sum += samples[i + ch];
        }
        mono.push_back(sum / channels);
    }
    
    result.samples = downsample(mono.data(), static_cast<int>(mono.size()), targetSamples);
    return result;
}

std::vector<float> WaveformGenerator::downsample(const float* input, int inputLength, int outputLength) {
    std::vector<float> output(outputLength, 0.0f);
    
    if (!input || inputLength <= 0 || outputLength <= 0) return output;
    
    float samplesPerOutput = static_cast<float>(inputLength) / outputLength;
    
    for (int i = 0; i < outputLength; ++i) {
        int start = static_cast<int>(i * samplesPerOutput);
        int end = static_cast<int>((i + 1) * samplesPerOutput);
        end = std::min(end, inputLength);
        
        float maxVal = 0.0f;
        for (int j = start; j < end; ++j) {
            maxVal = std::max(maxVal, std::abs(input[j]));
        }
        output[i] = maxVal;
    }
    
    return output;
}

} // namespace FreeEffect
