#include "../effect_registry.h"
#include "set_matte_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<SetMatteEffect> s_reg("Set Matte", "Channel");

SetMatteEffect::SetMatteEffect() {
    addParameter(EffectParameter::makeDropdown("useForMatte", "Use For Matte", {"Luminance", "Red", "Green", "Blue", "Alpha"}, 0));
    addParameter(EffectParameter::makeBool("invertMatte", "Invert Matte", false));
    addParameter(EffectParameter::makeBool("replaceWithMatte", "Replace With Matte", false));
}

std::unique_ptr<Effect> SetMatteEffect::clone() const {
    auto e = std::make_unique<SetMatteEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SetMatteEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int matteChannel = getDropdownParam("useForMatte");
    bool invert = getBoolParam("invertMatte");
    bool replace = getBoolParam("replaceWithMatte");

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float matte = 0;
            if (matteChannel == 0) matte = (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;
            else if (matteChannel == 1) matte = p[0] / 255.0f;
            else if (matteChannel == 2) matte = p[1] / 255.0f;
            else if (matteChannel == 3) matte = p[2] / 255.0f;
            else matte = p[3] / 255.0f;

            if (invert) matte = 1.0f - matte;
            uint8_t matteVal = static_cast<uint8_t>(std::clamp(matte * 255.0f, 0.0f, 255.0f));

            if (replace) {
                p[0] = matteVal;
                p[1] = matteVal;
                p[2] = matteVal;
            }
            p[3] = matteVal;
        }
    }
}

} // namespace FreeEffect
