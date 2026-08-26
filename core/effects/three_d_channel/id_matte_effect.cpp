#include "../effect_registry.h"
#include "id_matte_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<IDMatteEffect> s_reg("ID Matte", "3D Channel");

IDMatteEffect::IDMatteEffect() {
    addParameter(EffectParameter::makeInt("objectID", "Object/Material ID", 0, 100, 1));
    addParameter(EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 100.0, 0.0));
}

std::unique_ptr<Effect> IDMatteEffect::clone() const {
    auto e = std::make_unique<IDMatteEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void IDMatteEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int objID = getIntParam("objectID");
    float tolerance = getFloatParam("tolerance") / 100.0f;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float hash = std::fmod(std::sin(static_cast<float>(x * 127 + y * 311)) * 43758.5453f, 1.0f);
            int simID = static_cast<int>(hash * 100.0f);
            float match = (std::abs(simID - objID) <= static_cast<int>(tolerance * 10)) ? 1.0f : 0.0f;
            p[0] = p[1] = p[2] = static_cast<uint8_t>(match * 255.0f);
            p[3] = static_cast<uint8_t>(match * 255.0f);
        }
    }
}

} // namespace FreeEffect
