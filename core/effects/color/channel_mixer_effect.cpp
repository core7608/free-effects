#include "../effect_registry.h"
#include "channel_mixer_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ChannelMixerEffect> s_reg("Channel Mixer", "Color");

ChannelMixerEffect::ChannelMixerEffect() {
    addParameter(EffectParameter::makeFloat("red_in_red", "Red->Red", -200.0, 200.0, 100.0));
    addParameter(EffectParameter::makeFloat("red_in_green", "Green->Red", -200.0, 200.0, 0.0));
    addParameter(EffectParameter::makeFloat("red_in_blue", "Blue->Red", -200.0, 200.0, 0.0));
    addParameter(EffectParameter::makeFloat("green_in_red", "Red->Green", -200.0, 200.0, 0.0));
    addParameter(EffectParameter::makeFloat("green_in_green", "Green->Green", -200.0, 200.0, 100.0));
    addParameter(EffectParameter::makeFloat("green_in_blue", "Blue->Green", -200.0, 200.0, 0.0));
    addParameter(EffectParameter::makeFloat("blue_in_red", "Red->Blue", -200.0, 200.0, 0.0));
    addParameter(EffectParameter::makeFloat("blue_in_green", "Green->Blue", -200.0, 200.0, 0.0));
    addParameter(EffectParameter::makeFloat("blue_in_blue", "Blue->Blue", -200.0, 200.0, 100.0));
}

std::vector<ParameterGroup> ChannelMixerEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("red_in_red", "Red->Red", -200.0, 200.0, 100.0),
        EffectParameter::makeFloat("red_in_green", "Green->Red", -200.0, 200.0, 0.0),
        EffectParameter::makeFloat("red_in_blue", "Blue->Red", -200.0, 200.0, 0.0),
        EffectParameter::makeFloat("green_in_red", "Red->Green", -200.0, 200.0, 0.0),
        EffectParameter::makeFloat("green_in_green", "Green->Green", -200.0, 200.0, 100.0),
        EffectParameter::makeFloat("green_in_blue", "Blue->Green", -200.0, 200.0, 0.0),
        EffectParameter::makeFloat("blue_in_red", "Red->Blue", -200.0, 200.0, 0.0),
        EffectParameter::makeFloat("blue_in_green", "Green->Blue", -200.0, 200.0, 0.0),
        EffectParameter::makeFloat("blue_in_blue", "Blue->Blue", -200.0, 200.0, 100.0)
    }}};
}

std::unique_ptr<Effect> ChannelMixerEffect::clone() const {
    auto e = std::make_unique<ChannelMixerEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ChannelMixerEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double rr = getFloatParam("red_in_red") / 100.0;
    double rg = getFloatParam("red_in_green") / 100.0;
    double rb = getFloatParam("red_in_blue") / 100.0;
    double gr = getFloatParam("green_in_red") / 100.0;
    double gg = getFloatParam("green_in_green") / 100.0;
    double gb = getFloatParam("green_in_blue") / 100.0;
    double br = getFloatParam("blue_in_red") / 100.0;
    double bg = getFloatParam("blue_in_green") / 100.0;
    double bb = getFloatParam("blue_in_blue") / 100.0;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            double r = p[0] / 255.0;
            double g = p[1] / 255.0;
            double b = p[2] / 255.0;
            double nr = r * rr + g * rg + b * rb;
            double ng = r * gr + g * gg + b * gb;
            double nb = r * br + g * bg + b * bb;
            p[0] = static_cast<uint8_t>(std::clamp(nr * 255.0, 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(ng * 255.0, 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(nb * 255.0, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
