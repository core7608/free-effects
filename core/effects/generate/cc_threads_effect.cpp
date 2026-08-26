#include "../effect_registry.h"
#include "cc_threads_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<CCThreadsEffect> s_reg("CC Threads", "Generate");

CCThreadsEffect::CCThreadsEffect() {
    addParameter(EffectParameter::makeInt("numThreads", "Threads", 1, 100, 20));
    addParameter(EffectParameter::makeColor("color", "Color", {255.0, 255.0, 255.0, 1.0}));
    addParameter(EffectParameter::makeFloat("width", "Width", 0.5, 10.0, 1.0));
    addParameter(EffectParameter::makeFloat("phase", "Phase", 0.0, 360.0, 0.0));
}

std::unique_ptr<Effect> CCThreadsEffect::clone() const {
    auto e = std::make_unique<CCThreadsEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void CCThreadsEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int numThreads = getIntParam("numThreads");
    Color col = getColorParam("color");
    float width = getFloatParam("width");
    float phase = getFloatParam("phase") * 3.14159265f / 180.0f;

    for (int t = 0; t < numThreads; t++) {
        float startX = std::fmod(std::sin(static_cast<float>(t) * 127.1f + phase) * 43758.5453f, 1.0f) * buffer.width;
        float startY = std::fmod(std::sin(static_cast<float>(t) * 269.5f + phase) * 43758.5453f, 1.0f) * buffer.height;
        float endX = std::fmod(std::sin(static_cast<float>(t) * 419.2f + phase) * 43758.5453f, 1.0f) * buffer.width;
        float endY = std::fmod(std::sin(static_cast<float>(t) * 631.7f + phase) * 43758.5453f, 1.0f) * buffer.height;
        float dx = endX - startX, dy = endY - startY;
        float len = std::sqrt(dx*dx + dy*dy);
        if (len < 1) continue;
        float nx = -dy/len, ny = dx/len;

        for (int y = 0; y < buffer.height; y++) {
            for (int x = 0; x < buffer.width; x++) {
                float px = x - startX, py = y - startY;
                float proj = (px * dx + py * dy) / len;
                float cross = std::abs(px * nx + py * ny);
                if (proj >= 0 && proj <= len && cross < width) {
                    float alpha = 1.0f - cross / width;
                    uint8_t* p = buffer.pixelAt(x, y);
                    p[0] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[0] * (1-alpha) + col.r*alpha), 0.0, 255.0));
                    p[1] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[1] * (1-alpha) + col.g*alpha), 0.0, 255.0));
                    p[2] = static_cast<uint8_t>(std::clamp(static_cast<double>(p[2] * (1-alpha) + col.b*alpha), 0.0, 255.0));
                }
            }
        }
    }
}

} // namespace FreeEffect
