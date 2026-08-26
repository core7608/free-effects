#pragma once
#include "../effect.h"

namespace FreeEffect {

class InvertEffect : public Effect {
public:
    InvertEffect();
    std::string getName() const override { return "Invert"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
