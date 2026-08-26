#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>

namespace FreeEffect {

enum class AudioEffectType {
    EQ, Compressor, Limiter, Gate, Reverb, Delay, Chorus, Flanger,
    Phaser, Distortion, PitchShift, TimeStretch, AutoPan, Tremolo,
    Vibrato, LowPass, HighPass, BandPass, Notch
};

class AudioEffect {
public:
    virtual ~AudioEffect() = default;
    virtual AudioEffectType getType() const = 0;
    virtual std::string getName() const = 0;
    virtual void process(std::vector<float>& samples, int sampleRate) = 0;
    virtual void setParameter(const std::string& name, double value) = 0;
};

class EQEffect : public AudioEffect {
public:
    AudioEffectType getType() const override { return AudioEffectType::EQ; }
    std::string getName() const override { return "EQ"; }
    void process(std::vector<float>& samples, int sampleRate) override;
    void setParameter(const std::string& name, double value) override;
private:
    double m_lowFreq = 100, m_midFreq = 1000, m_highFreq = 8000;
    double m_lowGain = 0, m_midGain = 0, m_highGain = 0;
    double m_midQ = 1.0;
};

class CompressorEffect : public AudioEffect {
public:
    AudioEffectType getType() const override { return AudioEffectType::Compressor; }
    std::string getName() const override { return "Compressor"; }
    void process(std::vector<float>& samples, int sampleRate) override;
    void setParameter(const std::string& name, double value) override;
private:
    double m_threshold = -20, m_ratio = 4, m_attack = 10, m_release = 100, m_makeup = 0;
};

class LimiterEffect : public AudioEffect {
public:
    AudioEffectType getType() const override { return AudioEffectType::Limiter; }
    std::string getName() const override { return "Limiter"; }
    void process(std::vector<float>& samples, int sampleRate) override;
    void setParameter(const std::string& name, double value) override;
private:
    double m_threshold = -1, m_release = 50;
};

class GateEffect : public AudioEffect {
public:
    AudioEffectType getType() const override { return AudioEffectType::Gate; }
    std::string getName() const override { return "Gate"; }
    void process(std::vector<float>& samples, int sampleRate) override;
    void setParameter(const std::string& name, double value) override;
private:
    double m_threshold = -40, m_attack = 1, m_release = 50;
    double m_hold = 0, m_range = -100;
};

class ReverbEffect : public AudioEffect {
public:
    AudioEffectType getType() const override { return AudioEffectType::Reverb; }
    std::string getName() const override { return "Reverb"; }
    void process(std::vector<float>& samples, int sampleRate) override;
    void setParameter(const std::string& name, double value) override;
private:
    double m_roomSize = 0.5, m_damping = 0.5, m_wetDry = 0.3, m_predelay = 20;
    std::vector<float> m_buffer;
    int m_writePos = 0;
};

class DelayEffect : public AudioEffect {
public:
    AudioEffectType getType() const override { return AudioEffectType::Delay; }
    std::string getName() const override { return "Delay"; }
    void process(std::vector<float>& samples, int sampleRate) override;
    void setParameter(const std::string& name, double value) override;
private:
    double m_delay = 0.3, m_feedback = 0.4, m_wetDry = 0.3;
    std::vector<float> m_buffer;
    int m_writePos = 0;
};

class PitchShiftEffect : public AudioEffect {
public:
    AudioEffectType getType() const override { return AudioEffectType::PitchShift; }
    std::string getName() const override { return "Pitch Shift"; }
    void process(std::vector<float>& samples, int sampleRate) override;
    void setParameter(const std::string& name, double value) override;
private:
    double m_semitones = 0;
    double m_fineTune = 0;
};

class TimeStretchEffect : public AudioEffect {
public:
    AudioEffectType getType() const override { return AudioEffectType::TimeStretch; }
    std::string getName() const override { return "Time Stretch"; }
    void process(std::vector<float>& samples, int sampleRate) override;
    void setParameter(const std::string& name, double value) override;
private:
    double m_stretchFactor = 1.0;
    int m_windowSize = 2048;
};

class AutoPanEffect : public AudioEffect {
public:
    AudioEffectType getType() const override { return AudioEffectType::AutoPan; }
    std::string getName() const override { return "Auto Pan"; }
    void process(std::vector<float>& samples, int sampleRate) override;
    void setParameter(const std::string& name, double value) override;
private:
    double m_rate = 1.0, m_depth = 100, m_phase = 90;
    int m_samplePos = 0;
};

class TremoloEffect : public AudioEffect {
public:
    AudioEffectType getType() const override { return AudioEffectType::Tremolo; }
    std::string getName() const override { return "Tremolo"; }
    void process(std::vector<float>& samples, int sampleRate) override;
    void setParameter(const std::string& name, double value) override;
private:
    double m_rate = 5.0, m_depth = 50;
    int m_samplePos = 0;
};

class AudioProcessor {
public:
    static void convertToKeyframes(const std::vector<float>& samples, int sampleRate,
                                    double threshold, std::vector<double>& keyframeTimes,
                                    std::vector<double>& keyframeValues);

    static void pitchShift(std::vector<float>& samples, int sampleRate, double semitones);
    static void timeStretch(std::vector<float>& samples, int factor);
    static void normalize(std::vector<float>& samples, double targetDB = -1);
    static void fadeIn(std::vector<float>& samples, double duration, int sampleRate);
    static void fadeOut(std::vector<float>& samples, double duration, int sampleRate);
    static void pan(std::vector<float>& leftChannel, std::vector<float>& rightChannel, double panAmount);
    static void duckSidechain(std::vector<float>& music, const std::vector<float>& voice, double threshold, double ratio);

    static std::vector<float> applyEffectChain(const std::vector<float>& input, int sampleRate,
                                                const std::vector<std::unique_ptr<AudioEffect>>& effects);
};

} // namespace FreeEffect
