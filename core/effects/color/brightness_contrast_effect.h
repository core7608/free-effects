#pragma once
#include "../effect.h"

namespace FreeEffect {

class BrightnessContrastEffect : public Effect {
public:
    BrightnessContrastEffect();
    std::string getName() const override { return "Brightness & Contrast"; }
    std::string getCategory() const override { return "Color Correction"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
