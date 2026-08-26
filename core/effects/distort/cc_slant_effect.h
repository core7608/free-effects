#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCSlantEffect : public Effect {
public:
    CCSlantEffect();
    std::string getName() const override { return "CC Slant"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
