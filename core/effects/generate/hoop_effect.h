#pragma once
#include "../effect.h"

namespace FreeEffect {

class HoopEffect : public Effect {
public:
    HoopEffect();
    std::string getName() const override { return "Hoop"; }
    std::string getCategory() const override { return "Generate"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
