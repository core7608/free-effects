#pragma once
#include "../effect.h"

namespace FreeEffect {

class NoiseHLSEffect : public Effect {
public:
    NoiseHLSEffect();
    std::string getName() const override { return "Noise HLS"; }
    std::string getCategory() const override { return "Noise & Grain"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
