#pragma once
#include "../effect.h"

namespace FreeEffect {

class GalaxyEffect : public Effect {
public:
    GalaxyEffect();
    std::string getName() const override { return "Galaxy"; }
    std::string getCategory() const override { return "Simulation"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
