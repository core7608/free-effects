#pragma once
#include "../effect.h"

namespace FreeEffect {

class ReduceInterlaceFlickerEffect : public Effect {
public:
    ReduceInterlaceFlickerEffect();
    std::string getName() const override { return "Reduce Interlace Flicker"; }
    std::string getCategory() const override { return "Utility"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
