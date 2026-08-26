#pragma once
#include "../effect.h"

namespace FreeEffect {

class ChannelExtractEffect : public Effect {
public:
    ChannelExtractEffect();
    std::string getName() const override { return "Channel Extract"; }
    std::string getCategory() const override { return "3D Channel"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;
};

} // namespace FreeEffect
