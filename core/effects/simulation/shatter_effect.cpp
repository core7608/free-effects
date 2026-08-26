#include "../effect_registry.h"
#include "shatter_effect.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ShatterEffect> s_reg("Shatter", "Simulation");

ShatterEffect::ShatterEffect() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeFloat("force", "Force", 0.1, 5.0, 1.5));
    addParameter(EffectParameter::makeInt("pieces", "Piece Count", 4, 100, 20));
    addParameter(EffectParameter::makeFloat("gravity", "Gravity", 0.0, 5.0, 1.0));
    addParameter(EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5}));
}

std::vector<ParameterGroup> ShatterEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeFloat("force", "Force", 0.1, 5.0, 1.5),
        EffectParameter::makeInt("pieces", "Piece Count", 4, 100, 20),
        EffectParameter::makeFloat("gravity", "Gravity", 0.0, 5.0, 1.0),
        EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5})
    }}};
}

std::unique_ptr<Effect> ShatterEffect::clone() const {
    auto e = std::make_unique<ShatterEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ShatterEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    double force = getFloatParam("force");
    int pieces = getIntParam("pieces");
    double gravity = getFloatParam("gravity");
    Vec2 center = getVec2Param("center");
    double cx = center.x * buffer.width;
    double cy = center.y * buffer.height;
    if (progress < 0.001) return;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double dist = std::sqrt(dx * dx + dy * dy);
            double angle = std::atan2(dy, dx);
            int pieceId = static_cast<int>(std::abs(std::sin(angle * pieces * 0.5) * pieces * 127.1)) % pieces;
            double pieceAngle = (static_cast<double>(pieceId) / pieces) * 6.28318;
            double offset = dist * force * progress * (0.5 + 0.5 * std::sin(pieceId * 7.3));
            double gravY = gravity * progress * progress * 200.0;
            int srcX = static_cast<int>(std::round(x - std::cos(pieceAngle) * offset));
            int srcY = static_cast<int>(std::round(y - std::sin(pieceAngle) * offset - gravY * (pieceId % 2 == 0 ? 1.0 : -0.3)));
            if (srcX < 0 || srcX >= buffer.width || srcY < 0 || srcY >= buffer.height) {
                uint8_t* p = buffer.pixelAt(x, y);
                p[0] = p[1] = p[2] = 0;
                p[3] = 0;
                continue;
            }
            double fade = 1.0 - progress * 0.5;
            const uint8_t* src = tmp.pixelAt(srcX, srcY);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = static_cast<uint8_t>(src[0] * fade);
            dst[1] = static_cast<uint8_t>(src[1] * fade);
            dst[2] = static_cast<uint8_t>(src[2] * fade);
            dst[3] = static_cast<uint8_t>(src[3] * fade);
        }
    }
}

} // namespace FreeEffect
