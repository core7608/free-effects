#pragma once
#include "../effect.h"

namespace FreeEffect {

class WaveWarpEffect : public Effect {
public:
    WaveWarpEffect();
    std::string getName() const override { return "Wave Warp"; }
    std::string getCategory() const override { return "Distort"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
