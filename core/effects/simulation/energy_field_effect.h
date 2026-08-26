#pragma once
#include "../effect.h"

namespace FreeEffect {

class EnergyFieldEffect : public Effect {
public:
    EnergyFieldEffect();
    std::string getName() const override { return "Energy Field"; }
    std::string getCategory() const override { return "Simulation"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
