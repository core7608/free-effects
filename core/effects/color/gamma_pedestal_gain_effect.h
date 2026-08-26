#pragma once
#include "../effect.h"

namespace FreeEffect {

class GammaPedestalGainEffect : public Effect {
public:
    GammaPedestalGainEffect();
    std::string getName() const override { return "Gamma/Pedestal/Gain"; }
    std::string getCategory() const override { return "Color Correction"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
