#include "../effect_registry.h"
#include "iris_wipe_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<IrisWipeEffect> s_reg("Iris Wipe", "Transition");

IrisWipeEffect::IrisWipeEffect() {
    addParameter(EffectParameter::makeVec2("center", "Center", {0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("start", "Start", 0.0, 100.0, 0.0));
    addParameter(EffectParameter::makeFloat("end", "End", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeAngle("rotation", "Rotation", 0.0));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 500.0, 50.0));
}

std::unique_ptr<Effect> IrisWipeEffect::clone() const {
    auto e = std::make_unique<IrisWipeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void IrisWipeEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    if (buffer.width == 0 || buffer.height == 0) return;

    Vec2 ctr = getVec2Param("center");
    double startR = getFloatParam("start") / 100.0;
    double endR = getFloatParam("end") / 100.0;
    double feather = getFloatParam("feather");

    double cx = ctr.x * buffer.width;
    double cy = ctr.y * buffer.height;
    double maxR = std::sqrt(cx * cx + cy * cy) * 1.5;

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double dist = std::sqrt(dx * dx + dy * dy);

            double innerRadius = startR * maxR;
            double outerRadius = endR * maxR;
            double alpha = std::clamp((dist - innerRadius) /
                          std::max(outerRadius - innerRadius, 1.0), 0.0, 1.0);

            alpha = std::clamp((alpha * (outerRadius - innerRadius) + feather) /
                    (2.0 * feather + 1.0), 0.0, 1.0);

            if (alpha < 0.999) {
                uint8_t* p = buffer.pixelAt(x, y);
                p[0] = static_cast<uint8_t>(p[0] * alpha);
                p[1] = static_cast<uint8_t>(p[1] * alpha);
                p[2] = static_cast<uint8_t>(p[2] * alpha);
                p[3] = static_cast<uint8_t>(p[3] * alpha);
            }
        }
    }
}

} // namespace FreeEffect
