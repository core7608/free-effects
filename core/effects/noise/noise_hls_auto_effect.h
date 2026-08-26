#pragma once
#include "../effect.h"

namespace FreeEffect {

class NoiseHLSAutoEffect : public Effect {
public:
    NoiseHLSAutoEffect();
    std::string getName() const override { return "Noise HLS Auto"; }
    std::string getCategory() const override { return "Noise & Grain"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
