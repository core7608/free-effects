#pragma once
#include "../effect.h"

namespace FreeEffect {

class HueSaturationEffect : public Effect {
public:
    HueSaturationEffect();
    std::string getName() const override { return "Hue/Saturation"; }
    std::string getCategory() const override { return "Color Correction"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
