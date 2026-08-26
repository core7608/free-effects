#pragma once
#include "../effect.h"

namespace FreeEffect {

class OpticalFlareEffect : public Effect {
public:
    OpticalFlareEffect();
    std::string getName() const override { return "Optical Flare"; }
    std::string getCategory() const override { return "Plugin Effect"; }
    std::string getSubCategory() const override { return "Lens Effects"; }
    std::vector<ParameterGroup> getParameterGroups() const override;
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
