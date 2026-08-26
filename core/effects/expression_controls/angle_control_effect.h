#pragma once
#include "../effect.h"

namespace FreeEffect {

class AngleControlEffect : public Effect {
public:
    AngleControlEffect();
    std::string getName() const override { return "Angle Control"; }
    std::string getCategory() const override { return "Expression Controls"; }
    std::string getDescription() const override { return "An angle control for expressions"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
