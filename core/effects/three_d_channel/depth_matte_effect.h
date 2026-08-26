#pragma once
#include "../effect.h"

namespace FreeEffect {

class DepthMatteEffect : public Effect {
public:
    DepthMatteEffect();
    std::string getName() const override { return "Depth Matte"; }
    std::string getCategory() const override { return "3D Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
