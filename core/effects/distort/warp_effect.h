#pragma once
#include "../effect.h"

namespace FreeEffect {

class WarpEffect : public Effect {
public:
    WarpEffect();
    std::string getName() const override { return "Warp"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
