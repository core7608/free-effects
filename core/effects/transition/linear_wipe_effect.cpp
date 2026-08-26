#include "../effect_registry.h"
#include "linear_wipe_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<LinearWipeEffect> s_reg("Linear Wipe", "Transition");

LinearWipeEffect::LinearWipeEffect() {
    addParameter(EffectParameter::makeAngle("angle", "Wipe Angle", 90.0));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 500.0, 50.0));
    addParameter(EffectParameter::makeFloat("completion", "Transition Completion", 0.0, 100.0, 0.0));
}

std::unique_ptr<Effect> LinearWipeEffect::clone() const {
    auto e = std::make_unique<LinearWipeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void LinearWipeEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    if (buffer.width == 0 || buffer.height == 0) return;

    double angle = getAngleParam("angle") * 3.14159265 / 180.0;
    double feather = getFloatParam("feather");
    double completion = getFloatParam("completion") / 100.0;

    double cx = buffer.width * 0.5;
    double cy = buffer.height * 0.5;
    double maxDist = std::sqrt(cx * cx + cy * cy);

    double nx = std::cos(angle);
    double ny = std::sin(angle);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double proj = dx * nx + dy * ny;

            double normProj = (proj + maxDist) / (2.0 * maxDist);
            double edge = completion;
            double alpha = std::clamp((normProj - edge + feather / (2.0 * maxDist)) /
                          std::max(feather / maxDist, 0.001), 0.0, 1.0);

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
