#pragma once
#include "../effect.h"
#include <random>

namespace FreeEffect {

class NoiseEffect : public Effect {
public:
    NoiseEffect();
    std::string getName() const override { return "Noise"; }
    std::string getCategory() const override { return "Stylize"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
