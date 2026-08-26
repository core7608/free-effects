#include "audio_effects.h"
#include <algorithm>
#include <cmath>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace FreeEffect {

// ═══════════════════════════════════════════════════════════════════════
// EQ (3-band biquad)
// ═══════════════════════════════════════════════════════════════════════

struct BiquadCoeffs {
    double b0, b1, b2, a1, a2;
};

static BiquadCoeffs computePeakBiquad(double freq, double gain, double Q, int sampleRate) {
    double A = std::pow(10.0, gain / 40.0);
    double w0 = 2.0 * M_PI * freq / sampleRate;
    double alpha = std::sin(w0) / (2.0 * Q);
    double cosw0 = std::cos(w0);
    double b0 = 1.0 + alpha * A;
    double b1 = -2.0 * cosw0;
    double b2 = 1.0 - alpha * A;
    double a0 = 1.0 + alpha / A;
    double a1 = -2.0 * cosw0;
    double a2 = 1.0 - alpha / A;
    return { b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 };
}

static BiquadCoeffs computeLowShelf(double freq, double gain, int sampleRate) {
    double A = std::pow(10.0, gain / 40.0);
    double w0 = 2.0 * M_PI * freq / sampleRate;
    double alpha = std::sin(w0) / 2.0;
    double cosw0 = std::cos(w0);
    double b0 = A * ((A + 1.0) - (A - 1.0) * cosw0 + 2.0 * std::sqrt(A) * alpha);
    double b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosw0);
    double b2 = A * ((A + 1.0) - (A - 1.0) * cosw0 - 2.0 * std::sqrt(A) * alpha);
    double a0 = (A + 1.0) + (A - 1.0) * cosw0 + 2.0 * std::sqrt(A) * alpha;
    double a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosw0);
    double a2 = (A + 1.0) + (A - 1.0) * cosw0 - 2.0 * std::sqrt(A) * alpha;
    return { b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 };
}

static BiquadCoeffs computeHighShelf(double freq, double gain, int sampleRate) {
    double A = std::pow(10.0, gain / 40.0);
    double w0 = 2.0 * M_PI * freq / sampleRate;
    double alpha = std::sin(w0) / 2.0;
    double cosw0 = std::cos(w0);
    double b0 = A * ((A + 1.0) + (A - 1.0) * cosw0 + 2.0 * std::sqrt(A) * alpha);
    double b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosw0);
    double b2 = A * ((A + 1.0) + (A - 1.0) * cosw0 - 2.0 * std::sqrt(A) * alpha);
    double a0 = (A + 1.0) - (A - 1.0) * cosw0 + 2.0 * std::sqrt(A) * alpha;
    double a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosw0);
    double a2 = (A + 1.0) - (A - 1.0) * cosw0 - 2.0 * std::sqrt(A) * alpha;
    return { b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 };
}

static void applyBiquad(std::vector<float>& samples, const BiquadCoeffs& c) {
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    for (size_t i = 0; i < samples.size(); ++i) {
        double x0 = samples[i];
        double y0 = c.b0 * x0 + c.b1 * x1 + c.b2 * x2 - c.a1 * y1 - c.a2 * y2;
        x2 = x1; x1 = x0;
        y2 = y1; y1 = y0;
        samples[i] = static_cast<float>(std::clamp(y0, -1.0, 1.0));
    }
}

void EQEffect::process(std::vector<float>& samples, int sampleRate) {
    if (std::abs(m_lowGain) < 0.1 && std::abs(m_midGain) < 0.1 && std::abs(m_highGain) < 0.1) return;
    auto lowShelf = computeLowShelf(m_lowFreq, m_lowGain, sampleRate);
    auto peak = computePeakBiquad(m_midFreq, m_midGain, m_midQ, sampleRate);
    auto highShelf = computeHighShelf(m_highFreq, m_highGain, sampleRate);
    applyBiquad(samples, lowShelf);
    applyBiquad(samples, peak);
    applyBiquad(samples, highShelf);
}

void EQEffect::setParameter(const std::string& name, double value) {
    if (name == "lowFreq") m_lowFreq = value;
    else if (name == "midFreq") m_midFreq = value;
    else if (name == "highFreq") m_highFreq = value;
    else if (name == "lowGain") m_lowGain = value;
    else if (name == "midGain") m_midGain = value;
    else if (name == "highGain") m_highGain = value;
    else if (name == "midQ") m_midQ = value;
}

// ═══════════════════════════════════════════════════════════════════════
// Compressor
// ═══════════════════════════════════════════════════════════════════════

void CompressorEffect::process(std::vector<float>& samples, int sampleRate) {
    double threshLin = std::pow(10.0, m_threshold / 20.0);
    double attackCoeff = std::exp(-1.0 / (sampleRate * m_attack / 1000.0));
    double releaseCoeff = std::exp(-1.0 / (sampleRate * m_release / 1000.0));
    double makeupLin = std::pow(10.0, m_makeup / 20.0);
    double envelope = 0;

    for (size_t i = 0; i < samples.size(); ++i) {
        double absSample = std::abs(static_cast<double>(samples[i]));
        if (absSample > envelope) {
            envelope = attackCoeff * envelope + (1.0 - attackCoeff) * absSample;
        } else {
            envelope = releaseCoeff * envelope + (1.0 - releaseCoeff) * absSample;
        }

        double gain = 1.0;
        if (envelope > threshLin) {
            double overDB = 20.0 * std::log10(envelope / threshLin);
            double compressedDB = overDB * (1.0 / m_ratio);
            gain = std::pow(10.0, (compressedDB - overDB) / 20.0);
        }

        samples[i] = static_cast<float>(std::clamp(samples[i] * gain * makeupLin, -1.0, 1.0));
    }
}

void CompressorEffect::setParameter(const std::string& name, double value) {
    if (name == "threshold") m_threshold = value;
    else if (name == "ratio") m_ratio = value;
    else if (name == "attack") m_attack = value;
    else if (name == "release") m_release = value;
    else if (name == "makeup") m_makeup = value;
}

// ═══════════════════════════════════════════════════════════════════════
// Limiter
// ═══════════════════════════════════════════════════════════════════════

void LimiterEffect::process(std::vector<float>& samples, int sampleRate) {
    double threshLin = std::pow(10.0, m_threshold / 20.0);
    double releaseCoeff = std::exp(-1.0 / (sampleRate * m_release / 1000.0));
    double envelope = 0;

    for (size_t i = 0; i < samples.size(); ++i) {
        double absSample = std::abs(static_cast<double>(samples[i]));
        if (absSample > envelope) {
            envelope = absSample;
        } else {
            envelope = releaseCoeff * envelope + (1.0 - releaseCoeff) * absSample;
        }

        if (envelope > threshLin) {
            double gain = threshLin / envelope;
            samples[i] = static_cast<float>(samples[i] * gain);
        }
        samples[i] = static_cast<float>(std::clamp(static_cast<double>(samples[i]), -1.0, 1.0));
    }
}

void LimiterEffect::setParameter(const std::string& name, double value) {
    if (name == "threshold") m_threshold = value;
    else if (name == "release") m_release = value;
}

// ═══════════════════════════════════════════════════════════════════════
// Gate
// ═══════════════════════════════════════════════════════════════════════

void GateEffect::process(std::vector<float>& samples, int sampleRate) {
    double threshLin = std::pow(10.0, m_threshold / 20.0);
    double attackCoeff = std::exp(-1.0 / (sampleRate * m_attack / 1000.0));
    double releaseCoeff = std::exp(-1.0 / (sampleRate * m_release / 1000.0));
    double rangeLin = std::pow(10.0, m_range / 20.0);
    double holdSamples = m_hold / 1000.0 * sampleRate;
    double envelope = 0;
    int holdCounter = 0;
    bool gateOpen = false;

    for (size_t i = 0; i < samples.size(); ++i) {
        double absSample = std::abs(static_cast<double>(samples[i]));

        if (absSample > threshLin) {
            gateOpen = true;
            holdCounter = static_cast<int>(holdSamples);
            envelope = attackCoeff * envelope + (1.0 - attackCoeff) * absSample;
        } else {
            if (holdCounter > 0) {
                holdCounter--;
            } else {
                envelope = releaseCoeff * envelope + (1.0 - releaseCoeff) * absSample;
                if (envelope < threshLin * 0.5) gateOpen = false;
            }
        }

        if (!gateOpen) {
            samples[i] = static_cast<float>(samples[i] * rangeLin);
        }
    }
}

void GateEffect::setParameter(const std::string& name, double value) {
    if (name == "threshold") m_threshold = value;
    else if (name == "attack") m_attack = value;
    else if (name == "release") m_release = value;
    else if (name == "hold") m_hold = value;
    else if (name == "range") m_range = value;
}

// ═══════════════════════════════════════════════════════════════════════
// Reverb (Schroeder-style comb/allpass)
// ═══════════════════════════════════════════════════════════════════════

void ReverbEffect::process(std::vector<float>& samples, int sampleRate) {
    // Comb filter delays (in samples) based on room size
    static const int combDelays[] = { 1557, 1617, 1491, 1422, 1277, 1356 };
    static const int allpassDelays[] = { 556, 441, 341, 225 };
    static constexpr int numCombs = 6;
    static constexpr int numAllpass = 4;

    // Initialize buffer if needed
    int maxDelay = 1617;
    if (static_cast<int>(m_buffer.size()) < maxDelay + static_cast<int>(samples.size())) {
        m_buffer.resize(maxDelay + samples.size(), 0.0f);
        m_writePos = 0;
    }

    std::vector<float> wet(samples.size(), 0.0f);

    // Sum comb filter outputs
    double roomScale = m_roomSize * 0.3 + 0.7;
    double dampFactor = 1.0 - m_damping * 0.5;

    for (int c = 0; c < numCombs; ++c) {
        int delay = static_cast<int>(combDelays[c] * roomScale);
        double feedback = 0.84 - m_damping * 0.2;
        double combState = 0;

        for (size_t i = 0; i < samples.size(); ++i) {
            int readPos = (m_writePos - delay + static_cast<int>(m_buffer.size())) % static_cast<int>(m_buffer.size());
            double read = m_buffer[readPos];
            combState = read * feedback * dampFactor + combState * (1.0 - dampFactor * 0.1);
            m_buffer[(m_writePos + static_cast<int>(i)) % static_cast<int>(m_buffer.size())] =
                samples[i] + static_cast<float>(combState * 0.8);
            wet[i] += static_cast<float>(combState * (1.0 / numCombs));
        }
    }

    // Apply allpass filters
    for (int a = 0; a < numAllpass; ++a) {
        int delay = allpassDelays[a];
        for (size_t i = 0; i < wet.size(); ++i) {
            int readPos = (m_writePos + static_cast<int>(i) - delay + static_cast<int>(m_buffer.size()))
                          % static_cast<int>(m_buffer.size());
            float buf = m_buffer[readPos];
            float input = wet[i];
            wet[i] = -input + buf + 0.5f * input;
            m_buffer[(m_writePos + static_cast<int>(i)) % static_cast<int>(m_buffer.size())] = input;
        }
    }

    m_writePos = (m_writePos + static_cast<int>(samples.size())) % static_cast<int>(m_buffer.size());

    // Mix wet/dry
    for (size_t i = 0; i < samples.size(); ++i) {
        samples[i] = static_cast<float>(
            std::clamp(samples[i] * (1.0 - m_wetDry) + wet[i] * m_wetDry, -1.0, 1.0));
    }
}

void ReverbEffect::setParameter(const std::string& name, double value) {
    if (name == "roomSize") m_roomSize = value;
    else if (name == "damping") m_damping = value;
    else if (name == "wetDry") m_wetDry = value;
    else if (name == "predelay") m_predelay = value;
}

// ═══════════════════════════════════════════════════════════════════════
// Delay
// ═══════════════════════════════════════════════════════════════════════

void DelayEffect::process(std::vector<float>& samples, int sampleRate) {
    int delaySamples = static_cast<int>(m_delay * sampleRate);
    int bufSize = std::max(delaySamples + 1, static_cast<int>(44100 * 2));
    if (static_cast<int>(m_buffer.size()) < bufSize) {
        m_buffer.resize(bufSize, 0.0f);
        m_writePos = 0;
    }

    for (size_t i = 0; i < samples.size(); ++i) {
        int readPos = (m_writePos - delaySamples + static_cast<int>(m_buffer.size())) % static_cast<int>(m_buffer.size());
        float delayed = m_buffer[readPos];
        m_buffer[m_writePos] = samples[i] + delayed * static_cast<float>(m_feedback);
        m_writePos = (m_writePos + 1) % static_cast<int>(m_buffer.size());
        samples[i] = static_cast<float>(
            std::clamp(samples[i] * (1.0 - m_wetDry) + delayed * m_wetDry, -1.0, 1.0));
    }
}

void DelayEffect::setParameter(const std::string& name, double value) {
    if (name == "delay") m_delay = value;
    else if (name == "feedback") m_feedback = value;
    else if (name == "wetDry") m_wetDry = value;
}

// ═══════════════════════════════════════════════════════════════════════
// Pitch Shift (phase vocoder approach)
// ═══════════════════════════════════════════════════════════════════════

void PitchShiftEffect::process(std::vector<float>& samples, int sampleRate) {
    double totalShift = m_semitones + m_fineTune;
    if (std::abs(totalShift) < 0.01) return;

    double ratio = std::pow(2.0, totalShift / 12.0);
    if (std::abs(ratio - 1.0) < 0.001) return;

    int inputSize = static_cast<int>(samples.size());
    int outputSize = static_cast<int>(inputSize / ratio);
    std::vector<float> output(outputSize, 0.0f);

    // Simple overlap-add pitch shift
    int windowSize = 2048;
    int hopSize = windowSize / 4;
    int numWindows = (inputSize - windowSize) / hopSize + 1;
    if (numWindows < 1) numWindows = 1;

    for (int w = 0; w < numWindows; ++w) {
        int inputStart = w * hopSize;
        int outputStart = static_cast<int>(w * hopSize / ratio);

        for (int i = 0; i < windowSize && (inputStart + i) < inputSize; ++i) {
            int outIdx = outputStart + static_cast<int>(i / ratio);
            if (outIdx >= 0 && outIdx < outputSize) {
                // Hann window
                double hann = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / windowSize));
                output[outIdx] += static_cast<float>(samples[inputStart + i] * hann);
            }
        }
    }

    samples = std::move(output);
}

void PitchShiftEffect::setParameter(const std::string& name, double value) {
    if (name == "semitones") m_semitones = value;
    else if (name == "fineTune") m_fineTune = value;
}

// ═══════════════════════════════════════════════════════════════════════
// Time Stretch (WSOLA-like)
// ═══════════════════════════════════════════════════════════════════════

void TimeStretchEffect::process(std::vector<float>& samples, int sampleRate) {
    (void)sampleRate;
    if (std::abs(m_stretchFactor - 1.0) < 0.001) return;

    int inputSize = static_cast<int>(samples.size());
    int outputSize = static_cast<int>(inputSize * m_stretchFactor);
    std::vector<float> output(outputSize, 0.0f);

    int windowSize = m_windowSize;
    int readHop = windowSize;
    int writeHop = static_cast<int>(windowSize * m_stretchFactor);
    if (writeHop < 1) writeHop = 1;
    if (readHop < 1) readHop = 1;

    double scale = 1.0 / m_stretchFactor;
    int numWindows = inputSize / readHop;

    for (int w = 0; w < numWindows; ++w) {
        int readStart = w * readHop;
        int writeStart = static_cast<int>(w * writeHop * scale);

        for (int i = 0; i < windowSize; ++i) {
            int ri = readStart + i;
            int wi = writeStart + static_cast<int>(i * scale);
            if (ri < inputSize && wi >= 0 && wi < outputSize) {
                double hann = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / windowSize));
                output[wi] += static_cast<float>(samples[ri] * hann);
            }
        }
    }

    // Normalize to prevent clipping
    float maxVal = 0.001f;
    for (float v : output) maxVal = std::max(maxVal, std::abs(v));
    float normFactor = 1.0f / maxVal;
    for (auto& v : output) v *= normFactor;

    samples = std::move(output);
}

void TimeStretchEffect::setParameter(const std::string& name, double value) {
    if (name == "stretchFactor") m_stretchFactor = value;
    else if (name == "windowSize") m_windowSize = static_cast<int>(value);
}

// ═══════════════════════════════════════════════════════════════════════
// Auto Pan (LFO-based stereo panning)
// ═══════════════════════════════════════════════════════════════════════

void AutoPanEffect::process(std::vector<float>& samples, int sampleRate) {
    double phaseRad = m_phase * M_PI / 180.0;
    double depthNorm = m_depth / 100.0;

    for (size_t i = 0; i < samples.size(); ++i) {
        double phase = 2.0 * M_PI * m_rate * m_samplePos / sampleRate + phaseRad;
        double pan = std::sin(phase) * depthNorm;
        double leftGain = std::cos((pan + 1.0) * 0.25 * M_PI);
        double rightGain = std::sin((pan + 1.0) * 0.25 * M_PI);

        // For mono input, output remains mono but modulated
        samples[i] = static_cast<float>(samples[i] * std::max(leftGain, rightGain));
        m_samplePos++;
    }
}

void AutoPanEffect::setParameter(const std::string& name, double value) {
    if (name == "rate") m_rate = value;
    else if (name == "depth") m_depth = value;
    else if (name == "phase") m_phase = value;
}

// ═══════════════════════════════════════════════════════════════════════
// Tremolo (LFO volume modulation)
// ═══════════════════════════════════════════════════════════════════════

void TremoloEffect::process(std::vector<float>& samples, int sampleRate) {
    double depthNorm = m_depth / 100.0;

    for (size_t i = 0; i < samples.size(); ++i) {
        double phase = 2.0 * M_PI * m_rate * m_samplePos / sampleRate;
        double mod = 0.5 * (1.0 + depthNorm * std::sin(phase));
        samples[i] = static_cast<float>(samples[i] * mod);
        m_samplePos++;
    }
}

void TremoloEffect::setParameter(const std::string& name, double value) {
    if (name == "rate") m_rate = value;
    else if (name == "depth") m_depth = value;
}

// ═══════════════════════════════════════════════════════════════════════
// AudioProcessor utilities
// ═══════════════════════════════════════════════════════════════════════

void AudioProcessor::convertToKeyframes(const std::vector<float>& samples, int sampleRate,
                                         double threshold, std::vector<double>& keyframeTimes,
                                         std::vector<double>& keyframeValues) {
    keyframeTimes.clear();
    keyframeValues.clear();
    if (samples.empty() || sampleRate <= 0) return;

    // Simple onset detection
    float prevRMS = 0;
    int windowSize = sampleRate / 30; // ~33ms windows
    for (size_t i = 0; i + windowSize < samples.size(); i += windowSize) {
        float sum = 0;
        for (size_t j = 0; j < windowSize; ++j) {
            sum += samples[i + j] * samples[i + j];
        }
        float rms = std::sqrt(sum / windowSize);
        double time = static_cast<double>(i) / sampleRate;

        if (std::abs(rms - prevRMS) > threshold) {
            keyframeTimes.push_back(time);
            keyframeValues.push_back(static_cast<double>(rms));
        }
        prevRMS = rms;
    }
}

void AudioProcessor::pitchShift(std::vector<float>& samples, int sampleRate, double semitones) {
    PitchShiftEffect ps;
    ps.setParameter("semitones", semitones);
    ps.process(samples, sampleRate);
}

void AudioProcessor::timeStretch(std::vector<float>& samples, int factor) {
    if (factor <= 0) factor = 1;
    int inputSize = static_cast<int>(samples.size());
    int outputSize = inputSize * factor;
    std::vector<float> output(outputSize, 0.0f);

    for (int i = 0; i < outputSize; ++i) {
        int srcIdx = i / factor;
        if (srcIdx < inputSize) {
            output[i] = samples[srcIdx];
        }
    }
    samples = std::move(output);
}

void AudioProcessor::normalize(std::vector<float>& samples, double targetDB) {
    if (samples.empty()) return;
    float maxVal = 0.0f;
    for (float s : samples) maxVal = std::max(maxVal, std::abs(s));
    if (maxVal < 0.0001f) return;

    double targetLin = std::pow(10.0, targetDB / 20.0);
    float gain = static_cast<float>(targetLin / maxVal);
    for (auto& s : samples) s *= gain;
}

void AudioProcessor::fadeIn(std::vector<float>& samples, double duration, int sampleRate) {
    int fadeSamples = static_cast<int>(duration * sampleRate);
    fadeSamples = std::min(fadeSamples, static_cast<int>(samples.size()));
    for (int i = 0; i < fadeSamples; ++i) {
        float gain = static_cast<float>(i) / fadeSamples;
        samples[i] *= gain;
    }
}

void AudioProcessor::fadeOut(std::vector<float>& samples, double duration, int sampleRate) {
    int fadeSamples = static_cast<int>(duration * sampleRate);
    fadeSamples = std::min(fadeSamples, static_cast<int>(samples.size()));
    int start = static_cast<int>(samples.size()) - fadeSamples;
    for (int i = 0; i < fadeSamples; ++i) {
        float gain = 1.0f - static_cast<float>(i) / fadeSamples;
        samples[start + i] *= gain;
    }
}

void AudioProcessor::pan(std::vector<float>& leftChannel, std::vector<float>& rightChannel, double panAmount) {
    // panAmount: -1 (left) to 1 (right)
    double angle = (panAmount + 1.0) * 0.25 * M_PI;
    double leftGain = std::cos(angle);
    double rightGain = std::sin(angle);
    size_t len = std::min(leftChannel.size(), rightChannel.size());
    for (size_t i = 0; i < len; ++i) {
        leftChannel[i] = static_cast<float>(leftChannel[i] * leftGain);
        rightChannel[i] = static_cast<float>(rightChannel[i] * rightGain);
    }
}

void AudioProcessor::duckSidechain(std::vector<float>& music, const std::vector<float>& voice,
                                     double threshold, double ratio) {
    size_t len = std::min(music.size(), voice.size());
    double threshLin = std::pow(10.0, threshold / 20.0);
    double envelope = 0;
    double releaseCoeff = 0.999;

    for (size_t i = 0; i < len; ++i) {
        double absVoice = std::abs(static_cast<double>(voice[i]));
        if (absVoice > envelope) {
            envelope = absVoice;
        } else {
            envelope *= releaseCoeff;
        }

        if (envelope > threshLin) {
            double overDB = 20.0 * std::log10(envelope / threshLin);
            double gain = std::pow(10.0, -overDB * (1.0 - 1.0 / ratio) / 20.0);
            music[i] = static_cast<float>(music[i] * gain);
        }
    }
}

std::vector<float> AudioProcessor::applyEffectChain(const std::vector<float>& input, int sampleRate,
                                                      const std::vector<std::unique_ptr<AudioEffect>>& effects) {
    std::vector<float> output = input;
    for (const auto& fx : effects) {
        if (fx) {
            fx->process(output, sampleRate);
        }
    }
    return output;
}

} // namespace FreeEffect
