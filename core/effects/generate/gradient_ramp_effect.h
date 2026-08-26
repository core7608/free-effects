#pragma once
#include "../effect.h"

namespace FreeEffect {

class GradientRampEffect : public Effect {
public:
    GradientRampEffect();
    std::string getName() const override { return "Gradient Ramp"; }
    std::string getCategory() const override { return "Generate"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
