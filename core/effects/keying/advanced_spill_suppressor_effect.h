#pragma once
#include "../effect.h"

namespace FreeEffect {

class AdvancedSpillSuppressorEffect : public Effect {
public:
    AdvancedSpillSuppressorEffect();
    std::string getName() const override { return "Advanced Spill Suppressor"; }
    std::string getCategory() const override { return "Keying"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
