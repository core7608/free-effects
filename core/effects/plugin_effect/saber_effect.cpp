#include "../effect_registry.h"
#include "saber_effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

static EffectRegistrar<SaberEffect> s_reg("Saber", "Plugin Effect", "Video Copilot");

SaberEffect::SaberEffect() {
    addParameter(EffectParameter::makeVec2("start", "Start", Vec2{0.2, 0.5}));
    addParameter(EffectParameter::makeVec2("end", "End", Vec2{0.8, 0.5}));
    addParameter(EffectParameter::makeFloat("core_size", "Core Size", 1.0, 20.0, 4.0));
    addParameter(EffectParameter::makeFloat("glow_size", "Glow Size", 1.0, 100.0, 30.0));
    addParameter(EffectParameter::makeColor("core_color", "Core Color", Color{0.8, 0.9, 1.0, 1.0}));
    addParameter(EffectParameter::makeColor("glow_color", "Glow Color", Color{0.0, 0.3, 1.0, 0.8}));
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 3.0, 1.5));
    addParameter(EffectParameter::makeFloat("flicker", "Flicker", 0.0, 1.0, 0.2));
}

std::vector<ParameterGroup> SaberEffect::getParameterGroups() const {
    return {{getName(), {
        EffectParameter::makeVec2("start", "Start", Vec2{0.2, 0.5}),
        EffectParameter::makeVec2("end", "End", Vec2{0.8, 0.5}),
        EffectParameter::makeFloat("core_size", "Core Size", 1.0, 20.0, 4.0),
        EffectParameter::makeFloat("glow_size", "Glow Size", 1.0, 100.0, 30.0),
        EffectParameter::makeColor("core_color", "Core Color", Color{0.8, 0.9, 1.0, 1.0}),
        EffectParameter::makeColor("glow_color", "Glow Color", Color{0.0, 0.3, 1.0, 0.8}),
        EffectParameter::makeFloat("intensity", "Intensity", 0.0, 3.0, 1.5),
        EffectParameter::makeFloat("flicker", "Flicker", 0.0, 1.0, 0.2)
    }}};
}

std::unique_ptr<Effect> SaberEffect::clone() const {
    auto e = std::make_unique<SaberEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void SaberEffect::render(PixelBuffer& buffer, double time) {
    if (buffer.width == 0 || buffer.height == 0) return;
    Vec2 startP = getVec2Param("start");
    Vec2 endP = getVec2Param("end");
    double coreSz = getFloatParam("core_size");
    double glowSz = getFloatParam("glow_size");
    Color cc = getColorParam("core_color");
    Color gc = getColorParam("glow_color");
    double intensity = getFloatParam("intensity");
    double flickerAmt = getFloatParam("flicker");
    double flicker = 1.0 - flickerAmt * (0.5 + 0.5 * std::sin(time * 15.0) * std::sin(time * 7.3));
    intensity *= flicker;
    double x0 = startP.x * buffer.width;
    double y0 = startP.y * buffer.height;
    double x1 = endP.x * buffer.width;
    double y1 = endP.y * buffer.height;
    double lineLen = std::sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
    if (lineLen < 1) return;
    double nx = -(y1 - y0) / lineLen;
    double ny = (x1 - x0) / lineLen;
    int totalRad = static_cast<int>(std::ceil(glowSz * intensity));
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            double px = x - x0;
            double py = y - y0;
            double t = std::clamp((px * (x1 - x0) + py * (y1 - y0)) / (lineLen * lineLen), 0.0, 1.0);
            double projX = x0 + (x1 - x0) * t;
            double projY = y0 + (y1 - y0) * t;
            double perpDist = std::sqrt((x - projX) * (x - projX) + (y - projY) * (y - projY));
            double coreDist = coreSz * intensity;
            double glowDist = glowSz * intensity;
            double fa = 0.0;
            double fr = 0, fg = 0, fb = 0;
            if (perpDist < coreDist) {
                double coreFade = 1.0 - perpDist / coreDist;
                fa = coreFade;
                fr = cc.r * 255.0;
                fg = cc.g * 255.0;
                fb = cc.b * 255.0;
            } else if (perpDist < glowDist) {
                double glowFade = 1.0 - (perpDist - coreDist) / (glowDist - coreDist);
                glowFade = glowFade * glowFade;
                fa = glowFade * gc.a;
                fr = gc.r * 255.0;
                fg = gc.g * 255.0;
                fb = gc.b * 255.0;
            }
            if (fa < 0.01) continue;
            fa *= intensity;
            double endFade = 1.0;
            if (t < 0.1) endFade = t / 0.1;
            if (t > 0.9) endFade = (1.0 - t) / 0.1;
            fa *= endFade;
            uint8_t* p = buffer.pixelAt(x, y);
            double sa = p[3] / 255.0;
            double outA = sa + fa * (1.0 - sa);
            if (outA > 0.001) {
                p[0] = static_cast<uint8_t>(std::clamp((p[0] * sa + fr * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                p[1] = static_cast<uint8_t>(std::clamp((p[1] * sa + fg * fa * (1.0 - sa)) / outA, 0.0, 255.0));
                p[2] = static_cast<uint8_t>(std::clamp((p[2] * sa + fb * fa * (1.0 - sa)) / outA, 0.0, 255.0));
            }
            p[3] = static_cast<uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
