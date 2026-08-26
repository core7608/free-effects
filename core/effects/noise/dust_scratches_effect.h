#pragma once
#include "../effect.h"

namespace FreeEffect {

class DustScratchesEffect : public Effect {
public:
    DustScratchesEffect();
    std::string getName() const override { return "Dust & Scratches"; }
    std::string getCategory() const override { return "Noise & Grain"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
