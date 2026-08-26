#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCGlassWipeEffect : public Effect {
public:
    CCGlassWipeEffect();
    std::string getName() const override { return "CC Glass Wipe"; }
    std::string getCategory() const override { return "Transition"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
