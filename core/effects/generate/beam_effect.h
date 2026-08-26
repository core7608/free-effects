#pragma once
#include "../effect.h"

namespace FreeEffect {

class BeamEffect : public Effect {
public:
    BeamEffect();
    std::string getName() const override { return "Beam"; }
    std::string getCategory() const override { return "Generate"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
