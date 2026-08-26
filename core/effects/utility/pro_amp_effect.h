#pragma once
#include "../effect.h"

namespace FreeEffect {

class ProAmpEffect : public Effect {
public:
    ProAmpEffect();
    std::string getName() const override { return "Pro Amp"; }
    std::string getCategory() const override { return "Utility"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
