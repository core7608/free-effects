#pragma once
#include "../effect.h"

namespace FreeEffect {

class CausticsEffect : public Effect {
public:
    CausticsEffect();
    std::string getName() const override { return "Caustics"; }
    std::string getCategory() const override { return "Simulation"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
