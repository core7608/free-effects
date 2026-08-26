#pragma once
#include "../effect.h"

namespace FreeEffect {

class OffsetEffect : public Effect {
public:
    OffsetEffect();
    std::string getName() const override { return "Offset"; }
    std::string getCategory() const override { return "Distort"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
