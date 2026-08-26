#pragma once
#include "../effect.h"

namespace FreeEffect {

class SliderControlEffect : public Effect {
public:
    SliderControlEffect();
    std::string getName() const override { return "Slider Control"; }
    std::string getCategory() const override { return "Expression Controls"; }
    std::string getDescription() const override { return "A slider that can be linked to other properties"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
