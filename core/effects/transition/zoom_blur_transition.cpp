#include "../effect_registry.h"
#include "zoom_blur_transition.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<ZoomBlurTransition> s_reg("Zoom Blur Transition", "Transition");

ZoomBlurTransition::ZoomBlurTransition() {
    addParameter(EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0));
    addParameter(EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, 30.0));
    addParameter(EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5}));
}

std::vector<ParameterGroup> ZoomBlurTransition::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeFloat("progress", "Progress", 0.0, 1.0, 0.0),
        EffectParameter::makeFloat("amount", "Amount", 0.0, 100.0, 30.0),
        EffectParameter::makeVec2("center", "Center", Vec2{0.5, 0.5})
    }}};
}

std::unique_ptr<Effect> ZoomBlurTransition::clone() const {
    auto e = std::make_unique<ZoomBlurTransition>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void ZoomBlurTransition::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    double progress = getFloatParam("progress");
    double amt = getFloatParam("amount") * progress;
    Vec2 ctr = getVec2Param("center");
    double cx = ctr.x * buffer.width;
    double cy = ctr.y * buffer.height;
    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    tmp.data = buffer.data;
    int samples = 8;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double dx = x - cx;
            double dy = y - cy;
            double dist = std::sqrt(dx * dx + dy * dy);
            double maxDist = std::sqrt(cx * cx + cy * cy);
            double zoomFactor = dist / (maxDist + 1.0);
            double r = 0, g = 0, b = 0, a = 0;
            for (int s = 0; s < samples; s++) {
                double t = (static_cast<double>(s) / (samples - 1) - 0.5) * 2.0;
                double sampleDist = amt * zoomFactor * t;
                double sx = x + dx * sampleDist / (dist + 1.0) * 0.1;
                double sy = y + dy * sampleDist / (dist + 1.0) * 0.1;
                int isx = std::clamp(static_cast<int>(std::round(sx)), 0, buffer.width - 1);
                int isy = std::clamp(static_cast<int>(std::round(sy)), 0, buffer.height - 1);
                const uint8_t* p = tmp.pixelAt(isx, isy);
                r += p[0];
                g += p[1];
                b += p[2];
                a += p[3];
            }
            uint8_t* p = buffer.pixelAt(x, y);
            p[0] = static_cast<uint8_t>(std::clamp(r / samples, 0.0, 255.0));
            p[1] = static_cast<uint8_t>(std::clamp(g / samples, 0.0, 255.0));
            p[2] = static_cast<uint8_t>(std::clamp(b / samples, 0.0, 255.0));
            p[3] = static_cast<uint8_t>(std::clamp(a / samples, 0.0, 255.0));
        }
    }
    double fade = std::clamp(progress * 2.0, 0.0, 1.0);
    for (int i = 0; i < buffer.width * buffer.height; i++) {
        uint8_t* p = buffer.data.data() + i * 4;
        p[3] = static_cast<uint8_t>(p[3] * (1.0 - fade * 0.5));
    }
}

} // namespace FreeEffect
