#pragma once
#include "../effect.h"

namespace FreeEffect {

class RadialBlurEffect : public Effect {
public:
    RadialBlurEffect();
    std::string getName() const override { return "Radial Blur"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
