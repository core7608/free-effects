#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCLightWipeEffect : public Effect {
public:
    CCLightWipeEffect();
    std::string getName() const override { return "CC Light Wipe"; }
    std::string getCategory() const override { return "Transition"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
