#pragma once
#include "../effect.h"

namespace FreeEffect {

class AddGrainEffect : public Effect {
public:
    AddGrainEffect();
    std::string getName() const override { return "Add Grain"; }
    std::string getCategory() const override { return "Noise & Grain"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
