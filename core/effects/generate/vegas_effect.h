#pragma once
#include "../effect.h"

namespace FreeEffect {

class VegasEffect : public Effect {
public:
    VegasEffect();
    std::string getName() const override { return "Vegas"; }
    std::string getCategory() const override { return "Generate"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
