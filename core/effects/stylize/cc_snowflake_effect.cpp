#include "../effect_registry.h"
#include "cc_snowflake_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCSnowflakeEffect> s_reg("CC Snowflake", "Stylize");

CCSnowflakeEffect::CCSnowflakeEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("size", "Size", 10.0, 500.0, 100.0));
    addParameter(EffectParameter::makeInt("branches", "Branches", 3, 12, 6));
    addParameter(EffectParameter::makeAngle("rotation", "Rotation", 0.0));
    addParameter(EffectParameter::makeColor("color", "Color", {255.0, 255.0, 255.0, 1.0}));
}

std::unique_ptr<Effect> CCSnowflakeEffect::clone() const {
    auto e = std::make_unique<CCSnowflakeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCSnowflakeEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 center = getVec2Param("center");
    float size = getFloatParam("size");
    int branches = getIntParam("branches");
    float rot = getFloatParam("rotation") * 3.14159265f / 180.0f;
    Color col = getColorParam("color");

    float cx = center.x * buffer.width, cy = center.y * buffer.height;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float dx = x - cx, dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float angle = std::atan2(dy, dx) - rot;
            float segAngle = 2.0f * 3.14159265f / branches;
            float inSeg = std::fmod(angle, segAngle);
            if (inSeg < 0) inSeg += segAngle;
            float crossDist = std::abs(inSeg - segAngle * 0.5f);
            float armDist = crossDist * dist;
            float alongArm = dist;

            bool onArm = armDist < 3.0f && alongArm < size;
            bool onBranch = false;
            for (int b = 0; b < branches; b++) {
                float bAngle = b * segAngle + rot;
                float bDx = std::cos(bAngle), bDy = std::sin(bAngle);
                float proj = dx * bDx + dy * bDy;
                float cross = std::abs(dx * (-bDy) + dy * bDx);
                if (proj > 0 && proj < size && cross < 2.0f) onBranch = true;
                float subLen = size * 0.3f;
                float subPos = size * 0.5f;
                float sDx = proj - subPos * bDx, sDy = dy - subPos * bDy;
                if (proj > subPos * 0.8f && proj < (subPos + subLen) * 0.8f) {
                    float subProj = dx * (-bDy) + dy * bDx;
                    if (std::abs(subProj - size * 0.2f) < 1.5f) onBranch = true;
                }
            }

            if (onBranch || onArm) {
                uint8_t* p = buffer.pixelAt(x, y);
                float fade = 1.0f - dist / size;
                p[0] = static_cast<uint8_t>(std::min(255.0, static_cast<double>(p[0] + col.r * fade)));
                p[1] = static_cast<uint8_t>(std::min(255.0, static_cast<double>(p[1] + col.g * fade)));
                p[2] = static_cast<uint8_t>(std::min(255.0, static_cast<double>(p[2] + col.b * fade)));
            }
        }
    }
}

} // namespace FreeEffect
