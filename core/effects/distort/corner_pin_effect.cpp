#include "../effect_registry.h"
#include "corner_pin_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<CornerPinEffect> s_reg("Corner Pin", "Distort");

CornerPinEffect::CornerPinEffect() {
    addParameter(EffectParameter::makeVec2("upper_left", "Upper Left", Vec2{0.0, 0.0}));
    addParameter(EffectParameter::makeVec2("upper_right", "Upper Right", Vec2{1.0, 0.0}));
    addParameter(EffectParameter::makeVec2("lower_left", "Lower Left", Vec2{0.0, 1.0}));
    addParameter(EffectParameter::makeVec2("lower_right", "Lower Right", Vec2{1.0, 1.0}));
}

std::vector<ParameterGroup> CornerPinEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("upper_left", "Upper Left", Vec2{0.0, 0.0}),
        EffectParameter::makeVec2("upper_right", "Upper Right", Vec2{1.0, 0.0}),
        EffectParameter::makeVec2("lower_left", "Lower Left", Vec2{0.0, 1.0}),
        EffectParameter::makeVec2("lower_right", "Lower Right", Vec2{1.0, 1.0})
    }}};
}

std::unique_ptr<Effect> CornerPinEffect::clone() const {
    auto e = std::make_unique<CornerPinEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CornerPinEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    Vec2 ul = getVec2Param("upper_left");
    Vec2 ur = getVec2Param("upper_right");
    Vec2 ll = getVec2Param("lower_left");
    Vec2 lr = getVec2Param("lower_right");
    ul.x *= buffer.width; ul.y *= buffer.height;
    ur.x *= buffer.width; ur.y *= buffer.height;
    ll.x *= buffer.width; ll.y *= buffer.height;
    lr.x *= buffer.width; lr.y *= buffer.height;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double u = static_cast<double>(x) / buffer.width;
            double v = static_cast<double>(y) / buffer.height;
            double topX = ul.x + (ur.x - ul.x) * u;
            double topY = ul.y + (ur.y - ul.y) * u;
            double botX = ll.x + (lr.x - ll.x) * u;
            double botY = ll.y + (lr.y - ll.y) * u;
            double srcX = topX + (botX - topX) * v;
            double srcY = topY + (botY - topY) * v;
            int sx = std::clamp(static_cast<int>(std::round(srcX)), 0, buffer.width - 1);
            int sy = std::clamp(static_cast<int>(std::round(srcY)), 0, buffer.height - 1);
            const uint8_t* src = tmp.pixelAt(sx, sy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        }
    }
}

} // namespace FreeEffect
