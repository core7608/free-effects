#pragma once
#include "../effect.h"

namespace FreeEffect {

class ColorBalanceRGBEffect : public Effect {
public:
    ColorBalanceRGBEffect();
    std::string getName() const override { return "Color Balance (RGB)"; }
    std::string getCategory() const override { return "Color Correction"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
