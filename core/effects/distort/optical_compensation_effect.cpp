#include "../effect_registry.h"
#include "optical_compensation_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<OpticalCompensationEffect> s_reg("Optical Compensation", "Distort");

OpticalCompensationEffect::OpticalCompensationEffect() {
    addParameter(EffectParameter::makeFloat("amount", "Amount", -100.0, 100.0, 0.0));
    addParameter(EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5}));
    addParameter(EffectParameter::makeBool("reverse", "Reverse", false));
}

std::vector<ParameterGroup> OpticalCompensationEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("amount", "Amount", -100.0, 100.0, 0.0),
        EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5}),
        EffectParameter::makeBool("reverse", "Reverse", false)
    }}};
}

std::unique_ptr<Effect> OpticalCompensationEffect::clone() const {
    auto e = std::make_unique<OpticalCompensationEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void OpticalCompensationEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double amount = getFloatParam("amount") * 0.01;
    Vec2 ctr = getVec2Param("center");
    bool rev = getBoolParam("reverse");
    if (std::abs(amount) < 0.001) return;
    double cx = ctr.x * buffer.width;
    double cy = ctr.y * buffer.height;
    double maxR = std::sqrt(cx * cx + cy * cy);
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = (x - cx) / maxR;
            double dy = (y - cy) / maxR;
            double r2 = dx * dx + dy * dy;
            double r4 = r2 * r2;
            double distort = 1.0 + amount * r2 + amount * 0.5 * r4;
            if (rev) distort = 1.0 / distort;
            int sx = std::clamp(static_cast<int>(cx + dx * maxR * distort), 0, buffer.width - 1);
            int sy = std::clamp(static_cast<int>(cy + dy * maxR * distort), 0, buffer.height - 1);
            const uint8_t* src = tmp.pixelAt(sx, sy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        }
    }
}

} // namespace FreeEffect
