#pragma once
#include "../effect.h"

namespace FreeEffect {

class CheckboxControlEffect : public Effect {
public:
    CheckboxControlEffect();
    std::string getName() const override { return "Checkbox Control"; }
    std::string getCategory() const override { return "Expression Controls"; }
    std::string getDescription() const override { return "A checkbox control for expressions"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
