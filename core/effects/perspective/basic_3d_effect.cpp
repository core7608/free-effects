#include "../effect_registry.h"
#include "basic_3d_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<Basic3DEffect> s_reg("Basic 3D", "Perspective");

Basic3DEffect::Basic3DEffect() {
    addParameter(EffectParameter::makeAngle("swivel", "Swivel", 0.0));
    addParameter(EffectParameter::makeAngle("tilt", "Tilt", 0.0));
    addParameter(EffectParameter::makeFloat("distanceToImage", "Distance to Image", 0.0, 1000.0, 300.0));
}

std::unique_ptr<Effect> Basic3DEffect::clone() const {
    auto e = std::make_unique<Basic3DEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void Basic3DEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float swivel = getFloatParam("swivel") * 3.14159265f / 180.0f;
    float tilt = getFloatParam("tilt") * 3.14159265f / 180.0f;
    float dist = getFloatParam("distanceToImage");

    float cosS = std::cos(swivel), sinS = std::sin(swivel);
    float cosT = std::cos(tilt), sinT = std::sin(tilt);
    float cx = buffer.width / 2.0f, cy = buffer.height / 2.0f;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = (x - cx) / cx, dy = (y - cy) / cy;
            float rx = dx * cosS - dy * sinS * sinT;
            float ry = dy * cosT;
            float sx = cx + rx * cx;
            float sy = cy + ry * cy;
            int isx = std::clamp(static_cast<int>(sx), 0, buffer.width - 1);
            int isy = std::clamp(static_cast<int>(sy), 0, buffer.height - 1);
            const uint8_t* p = tmp.pixelAt(isx, isy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
        }
    }
}

} // namespace FreeEffect
