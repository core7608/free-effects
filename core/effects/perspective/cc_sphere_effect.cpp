#include "../effect_registry.h"
#include "cc_sphere_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCSphereEffect> s_reg("CC Sphere", "Perspective");

CCSphereEffect::CCSphereEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("radius", "Radius", 1.0, 100.0, 100.0));
    addParameter(EffectParameter::makeAngle("light", "Light Direction", -45.0));
    addParameter(EffectParameter::makeFloat("lightHeight", "Light Height", 0.0, 100.0, 50.0));
}

std::unique_ptr<Effect> CCSphereEffect::clone() const {
    auto e = std::make_unique<CCSphereEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCSphereEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 center = getVec2Param("center");
    float radius = getFloatParam("radius") / 100.0f;
    float lightDir = getFloatParam("light") * 3.14159265f / 180.0f;
    float lightH = getFloatParam("lightHeight") / 100.0f;

    float cx = center.x * buffer.width, cy = center.y * buffer.height;
    float r = radius * std::min(buffer.width, buffer.height) * 0.5f;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx, dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            uint8_t* dst = buffer.pixelAt(x, y);
            if (dist < r) {
                float z = std::sqrt(r * r - dx * dx - dy * dy);
                float nx2 = dx / r, ny2 = dy / r, nz2 = z / r;
                float lx = std::cos(lightDir) * lightH, ly = std::sin(lightDir) * lightH, lz = std::sqrt(1.0f - lightH * lightH);
                float shade = std::max(0.0f, nx2 * lx + ny2 * ly + nz2 * lz);
                float srcU = 0.5f + std::atan2(nx2, nz2) / (2.0f * 3.14159265f);
                float srcV = 0.5f - std::asin(ny2) / 3.14159265f;
                int isx = std::clamp(static_cast<int>(srcU * buffer.width), 0, buffer.width - 1);
                int isy = std::clamp(static_cast<int>(srcV * buffer.height), 0, buffer.height - 1);
                const uint8_t* src = tmp.pixelAt(isx, isy);
                dst[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(src[0] * shade), 0.0, 255.0));
                dst[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(src[1] * shade), 0.0, 255.0));
                dst[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(src[2] * shade), 0.0, 255.0));
                dst[3] = 255;
            } else {
                dst[0] = dst[1] = dst[2] = 0; dst[3] = 0;
            }
        }
    }
}

} // namespace FreeEffect
