#pragma once
#include "../effect.h"

namespace FreeEffect {

class VectorPaintEffect : public Effect {
public:
    VectorPaintEffect();
    std::string getName() const override { return "Vector Paint"; }
    std::string getCategory() const override { return "Paint"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
