#pragma once
#include "../effect.h"

namespace FreeEffect {

class LensDistortionEffect : public Effect {
public:
    LensDistortionEffect();
    std::string getName() const override { return "Lens Distortion"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
