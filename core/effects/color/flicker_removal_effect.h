#pragma once
#include "../effect.h"

namespace FreeEffect {

class FlickerRemovalEffect : public Effect {
public:
    FlickerRemovalEffect();
    std::string getName() const override { return "Flicker Removal"; }
    std::string getCategory() const override { return "Color Correction"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
