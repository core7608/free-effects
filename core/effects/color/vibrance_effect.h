#pragma once
#include "../effect.h"

namespace FreeEffect {

class VibranceEffect : public Effect {
public:
    VibranceEffect();
    std::string getName() const override { return "Vibrance"; }
    std::string getCategory() const override { return "Color Correction"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
