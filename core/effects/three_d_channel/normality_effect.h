#pragma once
#include "../effect.h"

namespace FreeEffect {

class NormalityEffect : public Effect {
public:
    NormalityEffect();
    std::string getName() const override { return "Normality"; }
    std::string getCategory() const override { return "3D Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
