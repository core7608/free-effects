#include "../effect_registry.h"
#include "iris_transition.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<IrisTransition> s_reg("Iris Transition", "Transition");

IrisTransition::IrisTransition() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5}));
    addParameter(EffectParameter::makeFloat("feather", "Feather", 0.0, 50.0, 5.0));
    addParameter(EffectParameter::makeInt("sides", "Sides", 3, 32, 6));
}

std::vector<ParameterGroup> IrisTransition::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5}),
        EffectParameter::makeFloat("feather", "Feather", 0.0, 50.0, 5.0),
        EffectParameter::makeInt("sides", "Sides", 3, 32, 6)
    }}};
}

std::unique_ptr<Effect> IrisTransition::clone() const {
    auto e = std::make_unique<IrisTransition>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void IrisTransition::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    Vec2 ctr = getVec2Param("center");
    double feather = getFloatParam("feather");
    int sides = getIntParam("sides");
    double cx = ctr.x * buffer.width;
    double cy = ctr.y * buffer.height;
    double maxR = std::sqrt(cx * cx + cy * cy) * 1.5;
    double radius = progress * maxR * 1.2;
    double angleStep = 6.28318 / sides;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double dist = std::sqrt(dx * dx + dy * dy);
            double angle = std::atan2(dy, dx);
            double polyR = maxR * 1.2;
            if (sides > 2) {
                double sectorAngle = std::fmod(angle + 3.14159, angleStep) - angleStep / 2.0;
                polyR = (radius * 0.5) / std::cos(sectorAngle);
                polyR = std::min(polyR, maxR * 1.2);
            } else {
                polyR = radius;
            }
            double edgeDist = polyR - dist;
            double alpha = std::clamp((edgeDist + feather) / (feather * 2.0 + 1.0), 0.0, 1.0);
            if (alpha > 0.999) continue;
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(p[0] * alpha);
            p[1] = static_cast<uint8_t>(p[1] * alpha);
            p[2] = static_cast<uint8_t>(p[2] * alpha);
            p[3] = static_cast<uint8_t>(p[3] * alpha);
        }
    }
}

} // namespace FreeEffect
