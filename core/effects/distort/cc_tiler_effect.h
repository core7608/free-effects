#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCTilerEffect : public Effect {
public:
    CCTilerEffect();
    std::string getName() const override { return "CC Tiler"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
