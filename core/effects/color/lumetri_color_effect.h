#pragma once
#include "../effect.h"

namespace FreeEffect {

class LumetriColorEffect : public Effect {
public:
    LumetriColorEffect();
    std::string getName() const override { return "Lumetri Color"; }
    std::string getCategory() const override { return "Color Correction"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
