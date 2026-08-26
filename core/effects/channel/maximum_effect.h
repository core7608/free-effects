#pragma once
#include "../effect.h"

namespace FreeEffect {

class MaximumEffect : public Effect {
public:
    MaximumEffect();
    std::string getName() const override { return "Maximum"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
