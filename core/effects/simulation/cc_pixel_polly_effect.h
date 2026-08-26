#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCPixelPollyEffect : public Effect {
public:
    CCPixelPollyEffect();
    std::string getName() const override { return "CC Pixel Polly"; }
    std::string getCategory() const override { return "Simulation"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
