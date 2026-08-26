#pragma once
#include "../effect.h"

namespace FreeEffect {

class ChannelCombinerEffect : public Effect {
public:
    ChannelCombinerEffect();
    std::string getName() const override { return "Channel Combiner"; }
    std::string getCategory() const override { return "Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
