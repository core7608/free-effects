#pragma once
#include "../effect.h"

namespace FreeEffect {

class MatchGrainEffect : public Effect {
public:
    MatchGrainEffect();
    std::string getName() const override { return "Match Grain"; }
    std::string getCategory() const override { return "Noise & Grain"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
