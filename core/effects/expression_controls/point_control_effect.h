#pragma once
#include "../effect.h"

namespace FreeEffect {

class PointControlEffect : public Effect {
public:
    PointControlEffect();
    std::string getName() const override { return "Point Control"; }
    std::string getCategory() const override { return "Expression Controls"; }
    std::string getDescription() const override { return "A 2D point control for expressions"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
