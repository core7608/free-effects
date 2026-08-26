#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCLensEffect : public Effect {
public:
    CCLensEffect();
    std::string getName() const override { return "CC Lens"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
