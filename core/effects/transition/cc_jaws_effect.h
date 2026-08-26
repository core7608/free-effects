#pragma once
#include "../effect.h"

namespace FreeEffect {

class CCJawsEffect : public Effect {
public:
    CCJawsEffect();
    std::string getName() const override { return "CC Jaws"; }
    std::string getCategory() const override { return "Transition"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
