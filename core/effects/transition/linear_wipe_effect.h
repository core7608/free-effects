#pragma once
#include "../effect.h"

namespace FreeEffect {

class LinearWipeEffect : public Effect {
public:
    LinearWipeEffect();
    std::string getName() const override { return "Linear Wipe"; }
    std::string getCategory() const override { return "Transition"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
