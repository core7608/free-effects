#include "../../math/math_constants.h"
#include "../effect_registry.h"
#include "mirror_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<MirrorEffect> s_reg("Mirror", "Distort");

MirrorEffect::MirrorEffect() {
    addParameter(EffectParameter::makeAngle("reflection_center", "Reflection Center", 0.0));
    addParameter(EffectParameter::makeAngle("reflection_angle", "Reflection Angle", 0.0));
    addParameter(EffectParameter::makeBool("reverse", "Reverse", false));
}

std::vector<ParameterGroup> MirrorEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeAngle("reflection_center", "Reflection Center", 0.0),
        EffectParameter::makeAngle("reflection_angle", "Reflection Angle", 0.0),
        EffectParameter::makeBool("reverse", "Reverse", false)
    }}};
}

std::unique_ptr<Effect> MirrorEffect::clone() const {
    auto e = std::make_unique<MirrorEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void MirrorEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double centerAng = getAngleParam("reflection_center") * M_PI / 180.0;
    double reflAng = getAngleParam("reflection_angle") * M_PI / 180.0;
    bool rev = getBoolParam("reverse");
    double cx = buffer.width / 2.0 + std::cos(centerAng) * buffer.width * 0.25;
    double cy = buffer.height / 2.0 + std::sin(centerAng) * buffer.height * 0.25;
    double nx = std::cos(reflAng);
    double ny = std::sin(reflAng);
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double proj = dx * nx + dy * ny;
            if (proj > 0 && !rev) continue;
            if (proj < 0 && rev) continue;
            double perpX = dx - nx * proj * 2;
            double perpY = dy - ny * proj * 2;
            int sx = std::clamp(static_cast<int>(cx + perpX), 0, buffer.width - 1);
            int sy = std::clamp(static_cast<int>(cy + perpY), 0, buffer.height - 1);
            const uint8_t* src = tmp.pixelAt(sx, sy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        }
    }
}

} // namespace FreeEffect
