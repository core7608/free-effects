#pragma once
#include "../effect.h"

namespace FreeEffect {

class EchoSpaceEffect : public Effect {
public:
    EchoSpaceEffect();
    std::string getName() const override { return "Echo Space"; }
    std::string getCategory() const override { return "Generate"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
