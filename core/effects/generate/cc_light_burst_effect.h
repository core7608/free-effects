#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCLightBurstEffect : public Effect {
public:
    CCLightBurstEffect();
    std::string getName() const override { return "CC Light Burst"; }
    std::string getCategory() const override { return "Generate"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
