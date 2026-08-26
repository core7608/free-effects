#pragma once
#include "../effect.h"

namespace FreeEffect {

class TurbulentNoiseEffect : public Effect {
public:
    TurbulentNoiseEffect();
    std::string getName() const override { return "Turbulent Noise"; }
    std::string getCategory() const override { return "Noise & Grain"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
