#pragma once
#include "../effect.h"

namespace FreeEffect {

class WaveEffect : public Effect {
public:
    WaveEffect();
    std::string getName() const override { return "Wave"; }
    std::string getCategory() const override { return "Simulation"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
