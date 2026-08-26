#pragma once
#include "../effect.h"

namespace FreeEffect {

class RemoveGrainEffect : public Effect {
public:
    RemoveGrainEffect();
    std::string getName() const override { return "Remove Grain"; }
    std::string getCategory() const override { return "Noise & Grain"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
