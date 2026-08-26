#pragma once
#include "../effect.h"

namespace FreeEffect {

class ChangeToColorEffect : public Effect {
public:
    ChangeToColorEffect();
    std::string getName() const override { return "Change To Color"; }
    std::string getCategory() const override { return "Color Correction"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
