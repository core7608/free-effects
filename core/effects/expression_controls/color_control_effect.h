#pragma once
#include "../effect.h"

namespace FreeEffect {

class ColorControlEffect : public Effect {
public:
    ColorControlEffect();
    std::string getName() const override { return "Color Control"; }
    std::string getCategory() const override { return "Expression Controls"; }
    std::string getDescription() const override { return "A color control for expressions"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
