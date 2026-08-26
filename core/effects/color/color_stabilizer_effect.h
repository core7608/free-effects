#pragma once
#include "../effect.h"
#include <vector>

namespace FreeEffect {

class ColorStabilizerEffect : public Effect {
public:
    ColorStabilizerEffect();
    std::string getName() const override { return "Color Stabilizer"; }
    std::string getCategory() const override { return "Color Correction"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
