#pragma once
#include "../effect.h"

namespace FreeEffect {

class SetMatteEffect : public Effect {
public:
    SetMatteEffect();
    std::string getName() const override { return "Set Matte"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
