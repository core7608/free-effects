#pragma once
#include "../effect.h"

namespace FreeEffect {

class ScribbleEffect : public Effect {
public:
    ScribbleEffect();
    std::string getName() const override { return "Scribble"; }
    std::string getCategory() const override { return "Generate"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
