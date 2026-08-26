#pragma once
#include "../effect.h"

namespace FreeEffect {

class ColorBalanceEffect : public Effect {
public:
    ColorBalanceEffect();
    std::string getName() const override { return "Color Balance"; }
    std::string getCategory() const override { return "Color Correction"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
