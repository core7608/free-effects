#pragma once
#include "../effect.h"

namespace FreeEffect {

class WarpStabilizerEffect : public Effect {
public:
    WarpStabilizerEffect();
    std::string getName() const override { return "Warp Stabilizer"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
