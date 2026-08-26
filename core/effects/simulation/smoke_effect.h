#pragma once
#include "../effect.h"

namespace FreeEffect {

class SmokeEffect : public Effect {
public:
    SmokeEffect();
    std::string getName() const override { return "Smoke"; }
    std::string getCategory() const override { return "Simulation"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
