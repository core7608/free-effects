#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCLightSweepEffect : public Effect {
public:
    CCLightSweepEffect();
    std::string getName() const override { return "CC Light Sweep"; }
    std::string getCategory() const override { return "Generate"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
