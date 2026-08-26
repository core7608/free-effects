#pragma once
#include "../effect.h"

namespace FreeEffect {

class ParticlePlaygroundEffect : public Effect {
public:
    ParticlePlaygroundEffect();
    std::string getName() const override { return "Particle Playground"; }
    std::string getCategory() const override { return "Simulation"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
