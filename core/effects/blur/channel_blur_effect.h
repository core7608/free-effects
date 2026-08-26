#pragma once
#include "../effect.h"

namespace FreeEffect {

class ChannelBlurEffect : public Effect {
public:
    ChannelBlurEffect();
    std::string getName() const override { return "Channel Blur"; }
    std::string getCategory() const override { return "Blur & Sharpen"; }
    void render(PixelBuffer& buffer, double time) override;
    std::unique_ptr<Effect> clone() const override;

private:
    void blurChannel(std::vector<uint8_t>& data, int w, int h, int ch, int radius);
};

} // namespace FreeEffect
