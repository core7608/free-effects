#include "audio_decoder.h"
#include <cstring>
#include <algorithm>

namespace FreeEffect {

AudioDecoder::AudioDecoder() {
}

AudioDecoder::~AudioDecoder() {
    close();
}

bool AudioDecoder::open(const std::string& filePath) {
    // Stub implementation - FFmpeg integration will be added later
    // For now, detect WAV files by extension and provide basic support
    m_isOpen = false;
    
    std::string ext = filePath.substr(filePath.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == "wav" || ext == "mp3" || ext == "ogg" || ext == "flac") {
        m_format.sampleRate = 44100;
        m_format.channels = 2;
        m_format.bitsPerSample = 16;
        m_duration = 0.0;
        m_isOpen = true;
    }
    
    return m_isOpen;
}

void AudioDecoder::close() {
    m_isOpen = false;
    m_duration = 0.0;
}

int AudioDecoder::decode(float* outputBuffer, int maxSamples) {
    if (!m_isOpen || !outputBuffer) return 0;
    
    // Stub - returns silence
    std::memset(outputBuffer, 0, maxSamples * m_format.channels * sizeof(float));
    return 0;
}

bool AudioDecoder::seek(double timeInSeconds) {
    if (!m_isOpen) return false;
    return true;
}

} // namespace FreeEffect
