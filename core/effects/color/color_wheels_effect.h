#pragma once
#include "../effect.h"

namespace FreeEffect {

class ColorWheelsEffect : public Effect {
public:
    ColorWheelsEffect();
    std::string getName() const override { return "Color Wheels"; }
    std::string getCategory() const override { return "Color Correction"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
