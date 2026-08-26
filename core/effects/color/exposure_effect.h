#pragma once
#include "../effect.h"

namespace FreeEffect {

class ExposureEffect : public Effect {
public:
    ExposureEffect();
    std::string getName() const override { return "Exposure"; }
    std::string getCategory() const override { return "Color Correction"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
