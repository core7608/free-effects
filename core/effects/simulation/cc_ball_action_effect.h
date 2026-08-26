#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCBallActionEffect : public Effect {
public:
    CCBallActionEffect();
    std::string getName() const override { return "CC Ball Action"; }
    std::string getCategory() const override { return "Simulation"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
