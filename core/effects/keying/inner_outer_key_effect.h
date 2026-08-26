#pragma once
#include "../effect.h"

namespace FreeEffect {

class InnerOuterKeyEffect : public Effect {
public:
    InnerOuterKeyEffect();
    std::string getName() const override { return "Inner/Outer Key"; }
    std::string getCategory() const override { return "Keying"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
