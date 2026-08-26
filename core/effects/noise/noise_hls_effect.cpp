#include "../effect_registry.h"
#include "noise_hls_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<NoiseHLSEffect> s_reg("Noise HLS", "Noise & Grain");

NoiseHLSEffect::NoiseHLSEffect() {
    addParameter(EffectParameter::makeDropdown("noiseType", "Noise Type", {"Uniform", "Grainy"}, 0));
    addParameter(EffectParameter::makeFloat("hue", "Hue", 0.0, 360.0, 0.0));
    addParameter(EffectParameter::makeFloat("lightness", "Lightness", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("saturation", "Saturation", 0.0, 100.0, 0.0));
}

std::unique_ptr<Effect> NoiseHLSEffect::clone() const {
    auto e = std::make_unique<NoiseHLSEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void NoiseHLSEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float hue = getFloatParam("hue") / 360.0f;
    float light = getFloatParam("lightness") / 100.0f;
    float sat = getFloatParam("saturation") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float r = p[0]/255.0f, g = p[1]/255.0f, b = p[2]/255.0f;
            float cmax = std::max({r,g,b}), cmin = std::min({r,g,b});
            float h = 0, s = 0, l = (cmax+cmin)/2.0f;
            float delta = cmax - cmin;
            if (delta > 0.001f) {
                s = l > 0.5f ? delta/(2.0f-cmax-cmin) : delta/(cmax+cmin);
                if (cmax == r) h = (g-b)/delta + (g < b ? 6.0f : 0.0f);
                else if (cmax == g) h = (b-r)/delta + 2.0f;
                else h = (r-g)/delta + 4.0f;
                h /= 6.0f;
            }
            float noise = (std::fmod(std::sin(x*12.9898f+y*78.233f)*43758.5453f, 1.0f) - 0.5f);
            h = std::fmod(h + hue * noise + 1.0f, 1.0f);
            s = std::clamp(s + sat * std::abs(noise), 0.0f, 1.0f);
            l = std::clamp(l + light * noise, 0.0f, 1.0f);
            float q = l < 0.5f ? l*(1.0f+s) : l+s-l*s;
            float p2 = 2.0f*l-q;
            auto h2r = [](float p,float q,float t){if(t<0)t+=1;if(t>1)t-=1;if(t<1.0f/6)return p+(q-p)*6*t;if(t<.5f)return q;if(t<2.0f/3)return p+(q-p)*(2.0f/3-t)*6;return p;};
            p[0] = static_cast<uint8_t>(std::clamp(h2r(p2,q,h+1.0f/3)*255, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp(h2r(p2,q,h)*255, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp(h2r(p2,q,h-1.0f/3)*255, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
