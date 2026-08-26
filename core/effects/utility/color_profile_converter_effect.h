#pragma once
#include "../effect.h"

namespace FreeEffect {

class ColorProfileConverterEffect : public Effect {
public:
    ColorProfileConverterEffect();
    std::string getName() const override { return "Color Profile Converter"; }
    std::string getCategory() const override { return "Utility"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
