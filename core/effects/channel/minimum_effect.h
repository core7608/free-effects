#pragma once
#include "../effect.h"

namespace FreeEffect {

class MinimumEffect : public Effect {
public:
    MinimumEffect();
    std::string getName() const override { return "Minimum"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
