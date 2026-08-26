#pragma once
#include "../effect.h"

namespace FreeEffect {

class ColorHalftoneEffect : public Effect {
public:
    ColorHalftoneEffect();
    std::string getName() const override { return "Color Halftone"; }
    std::string getCategory() const override { return "Stylize"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
