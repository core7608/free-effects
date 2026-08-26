#pragma once
#include "../effect.h"

namespace FreeEffect {

class SimpleChokerEffect : public Effect {
public:
    SimpleChokerEffect();
    std::string getName() const override { return "Simple Choker"; }
    std::string getCategory() const override { return "Keying"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
