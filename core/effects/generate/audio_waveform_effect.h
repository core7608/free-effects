#pragma once
#include "../effect.h"

namespace FreeEffect {

class AudioWaveformEffect : public Effect {
public:
    AudioWaveformEffect();
    std::string getName() const override { return "Audio Waveform"; }
    std::string getCategory() const override { return "Generate"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
