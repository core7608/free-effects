#include "../effect_registry.h"
#include "emboss_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<EmbossEffect> s_reg("Emboss", "Stylize");

EmbossEffect::EmbossEffect() {
    addParameter(EffectParameter::makeAngle("direction", "Direction", 135.0));
    addParameter(EffectParameter::makeFloat("height", "Height", 0.0, 10.0, 2.0));
    addParameter(EffectParameter::makeFloat("amount", "Amount", 0.0, 5.0, 1.0));
}

std::vector<ParameterGroup> EmbossEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeAngle("direction", "Direction", 135.0),
        EffectParameter::makeFloat("height", "Height", 0.0, 10.0, 2.0),
        EffectParameter::makeFloat("amount", "Amount", 0.0, 5.0, 1.0)
    }}};
}

std::unique_ptr<Effect> EmbossEffect::clone() const {
    auto e = std::make_unique<EmbossEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void EmbossEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double dir = getAngleParam("direction") * M_PI / 180.0;
    double h = getFloatParam("height");
    double amount = getFloatParam("amount");
    int offX = static_cast<int>(std::round(std::cos(dir)));
    int offY = static_cast<int>(std::round(std::sin(dir)));
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 1; y < buffer.height - 1; y++) {
        for (int x = 1; x < buffer.width - 1; x++) {
            const uint8_t* c = tmp.pixelAt(x, y);
            const uint8_t* n = tmp.pixelAt(std::clamp(x + offX, 0, buffer.width - 1),
                                           std::clamp(y + offY, 0, buffer.height - 1));
            uint8_t* p = buffer.pixelAt(x, y);
            double lumC = c[0] * 0.299 + c[1] * 0.587 + c[2] * 0.114;
            double lumN = n[0] * 0.299 + n[1] * 0.587 + n[2] * 0.114;
            double diff = (lumC - lumN) * h * amount;
            double gray = 128.0 + diff;
            p[0] = static_cast<uint8_t>(std::clamp(gray, 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(gray, 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(gray, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
