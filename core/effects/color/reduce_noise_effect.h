#pragma once
#include "../effect.h"

namespace FreeEffect {

class ReduceNoiseEffect : public Effect {
public:
    ReduceNoiseEffect();
    std::string getName() const override { return "Reduce Noise"; }
    std::string getCategory() const override { return "Color Correction"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
