#pragma once
#include "../effect.h"

namespace FreeEffect {

class RefineSoftMatteEffect : public Effect {
public:
    RefineSoftMatteEffect();
    std::string getName() const override { return "Refine Soft Matte"; }
    std::string getCategory() const override { return "Keying"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
