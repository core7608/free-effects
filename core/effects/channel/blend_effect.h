#pragma once
#include "../effect.h"

namespace FreeEffect {

class BlendEffect : public Effect {
public:
    BlendEffect();
    std::string getName() const override { return "Blend"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
