#pragma once
#include "../effect.h"

namespace FreeEffect {

class ChannelMixerEffect : public Effect {
public:
    ChannelMixerEffect();
    std::string getName() const override { return "Channel Mixer"; }
    std::string getCategory() const override { return "Color"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
