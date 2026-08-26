#pragma once
#include "../effect.h"

namespace FreeEffect {

class MinimaxEffect : public Effect {
public:
    MinimaxEffect();
    std::string getName() const override { return "Minimax"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
