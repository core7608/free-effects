#pragma once
#include "../effect.h"

namespace FreeEffect {

class LutBuddyEffect : public Effect {
public:
    LutBuddyEffect();
    std::string getName() const override { return "LUT Buddy"; }
    std::string getCategory() const override { return "Color Correction"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
