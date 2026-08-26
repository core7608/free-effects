#pragma once
#include "../effect.h"

namespace FreeEffect {

class IrisWipeEffect : public Effect {
public:
    IrisWipeEffect();
    std::string getName() const override { return "Iris Wipe"; }
    std::string getCategory() const override { return "Transition"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
