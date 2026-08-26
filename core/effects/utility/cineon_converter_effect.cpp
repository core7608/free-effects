#include "../effect_registry.h"
#include "cineon_converter_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<CineonConverterEffect> s_reg("Cineon Converter", "Utility");

CineonConverterEffect::CineonConverterEffect() {
    addParameter(EffectParameter::makeFloat("blackPoint", "Black Point", 0.0, 1023.0, 95.0));
    addParameter(EffectParameter::makeFloat("whitePoint", "White Point", 0.0, 1023.0, 685.0));
    addParameter(EffectParameter::makeFloat("gamma", "Gamma", 0.1, 10.0, 1.7));
}

std::unique_ptr<Effect> CineonConverterEffect::clone() const {
    auto e = std::make_unique<CineonConverterEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CineonConverterEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    double black = getFloatParam("blackPoint");
    double white = getFloatParam("whitePoint");
    double gamma = getFloatParam("gamma");

    double range = white - black;
    if (range <= 0.0) range = 1.0;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            for (int c = 0; c < 3; c++) {
                double val = p[c] / 255.0 * 1023.0;
                double linear = std::pow(std::max(0.0, (val - black) / range), gamma);
                p[c] = static_cast<uint8_t>(std::clamp(linear * 255.0, 0.0, 255.0));
            }
        }
    }
}

} // namespace FreeEffect
