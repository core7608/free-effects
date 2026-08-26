#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCScaleWipeEffect : public Effect {
public:
    CCScaleWipeEffect();
    std::string getName() const override { return "CC Scale Wipe"; }
    std::string getCategory() const override { return "Transition"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
