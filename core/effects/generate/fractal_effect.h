#pragma once
#include "../effect.h"

namespace FreeEffect {

class FractalEffect : public Effect {
public:
    FractalEffect();
    std::string getName() const override { return "Fractal"; }
    std::string getCategory() const override { return "Generate"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
