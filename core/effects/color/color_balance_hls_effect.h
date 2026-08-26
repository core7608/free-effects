#pragma once
#include "../effect.h"

namespace FreeEffect {

class ColorBalanceHLSEffect : public Effect {
public:
    ColorBalanceHLSEffect();
    std::string getName() const override { return "Color Balance (HLS)"; }
    std::string getCategory() const override { return "Color Correction"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
