#pragma once
#include "../effect.h"

namespace FreeEffect {

class KeylightEffect : public Effect {
public:
    KeylightEffect();
    std::string getName() const override { return "Keylight"; }
    std::string getCategory() const override { return "Keying"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
