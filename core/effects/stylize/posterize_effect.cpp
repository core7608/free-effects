#include "../effect_registry.h"
#include "posterize_effect.h"
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<PosterizeEffect> s_reg("Posterize", "Stylize");

PosterizeEffect::PosterizeEffect() {
    addParameter(EffectParameter::makeInt("levels", "Levels", 2, 256, 4));
}

std::vector<ParameterGroup> PosterizeEffect::getParameterGroups() const {
    return {{getName(), {EffectParameter::makeInt("levels", "Levels", 2, 256, false)}}};
}

std::unique_ptr<Effect> PosterizeEffect::clone() const {
    auto e = std::make_unique<PosterizeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void PosterizeEffect::render(PixelBuffer& buffer, double time) {
    int levels = std::max(getIntParam("levels"), 2);
    float step = 255.0f / (levels - 1);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            for (int c = 0; c < 3; c++) {
                p[c] = static_cast<uint8_t>(std::round(p[c] / step) * step);
                p[c] = std::min(p[c], static_cast<uint8_t>(255));
            }
        }
    }
}

} // namespace FreeEffect
