#pragma once
#include "../effect.h"

namespace FreeEffect {

class SolidCompositeEffect : public Effect {
public:
    SolidCompositeEffect();
    std::string getName() const override { return "Solid Composite"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
