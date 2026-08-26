#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCCompositeEffect : public Effect {
public:
    CCCompositeEffect();
    std::string getName() const override { return "CC Composite"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
