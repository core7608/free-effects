#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCBendItEffect : public Effect {
public:
    CCBendItEffect();
    std::string getName() const override { return "CC Bend It"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
