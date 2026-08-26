#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCWedgeWipeEffect : public Effect {
public:
    CCWedgeWipeEffect();
    std::string getName() const override { return "CC Wedge Wipe"; }
    std::string getCategory() const override { return "Transition"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
