#include "../effect_registry.h"
#include "liquify_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<LiquifyEffect> s_reg("Liquify", "Distort");

LiquifyEffect::LiquifyEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("amount", "Amount", -100.0, 100.0, 20.0));
    addParameter(EffectParameter::makeFloat("radius", "Radius", 10.0, 500.0, 100.0));
    addParameter(EffectParameter::makeInt("mode", "Mode", 0, 2, 0));
}

std::vector<ParameterGroup> LiquifyEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5}),
        EffectParameter::makeFloat("amount", "Amount", -100.0, 100.0, 20.0),
        EffectParameter::makeFloat("radius", "Radius", 10.0, 500.0, 100.0),
        EffectParameter::makeInt("mode", "Mode", 0, 2, 0)
    }}};
}

std::unique_ptr<Effect> LiquifyEffect::clone() const {
    auto e = std::make_unique<LiquifyEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void LiquifyEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    Vec2 ctr = getVec2Param("center");
    double amount = getFloatParam("amount");
    double radius = getFloatParam("radius");
    int mode = getIntParam("mode");
    double cx = ctr.x * buffer.width;
    double cy = ctr.y * buffer.height;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double dist = std::sqrt(dx * dx + dy * dy);
            if (dist > radius || dist < 0.001) {
                const uint8_t* src = tmp.pixelAt(x, y);
                uint8_t* dst = buffer.pixelAt(x, y);
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
                continue;
            }
            double falloff = 1.0 - dist / radius;
            falloff = falloff * falloff;
            double push = amount * falloff;
            int sx, sy;
            if (mode == 0) {
                sx = std::clamp(static_cast<int>(x + dx / dist * push), 0, buffer.width - 1);
                sy = std::clamp(static_cast<int>(y + dy / dist * push), 0, buffer.height - 1);
            } else if (mode == 1) {
                double angle = std::atan2(dy, dx) + push * 0.01;
                double newDist = dist + push * 0.1;
                sx = std::clamp(static_cast<int>(cx + std::cos(angle) * newDist), 0, buffer.width - 1);
                sy = std::clamp(static_cast<int>(cy + std::sin(angle) * newDist), 0, buffer.height - 1);
            } else {
                double angle = std::atan2(dy, dx);
                double newDist = dist - push * 0.2;
                sx = std::clamp(static_cast<int>(cx + std::cos(angle) * newDist), 0, buffer.width - 1);
                sy = std::clamp(static_cast<int>(cy + std::sin(angle) * newDist), 0, buffer.height - 1);
            }
            const uint8_t* src = tmp.pixelAt(sx, sy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        }
    }
}

} // namespace FreeEffect
