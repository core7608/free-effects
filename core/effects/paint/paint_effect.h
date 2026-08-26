#pragma once
#include "../effect.h"

namespace FreeEffect {

class PaintEffect : public Effect {
public:
    PaintEffect();
    std::string getName() const override { return "Paint"; }
    std::string getCategory() const override { return "Paint"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
