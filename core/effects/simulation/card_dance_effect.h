#pragma once
#include "../effect.h"

namespace FreeEffect {

class CardDanceEffect : public Effect {
public:
    CardDanceEffect();
    std::string getName() const override { return "Card Dance"; }
    std::string getCategory() const override { return "Simulation"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
