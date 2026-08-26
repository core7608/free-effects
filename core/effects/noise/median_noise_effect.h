#pragma once
#include "../effect.h"

namespace FreeEffect {

class MedianNoiseEffect : public Effect {
public:
    MedianNoiseEffect();
    std::string getName() const override { return "Median"; }
    std::string getCategory() const override { return "Noise & Grain"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
