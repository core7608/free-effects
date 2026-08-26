#include "../effect_registry.h"
#include "paint_bucket_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<PaintBucketEffect> s_reg("Paint Bucket", "Generate");

PaintBucketEffect::PaintBucketEffect() {
    addParameter(EffectParameter::makeVec2("fillPoint", "Fill Point", {0.5, 0.5}));
    addParameter(EffectParameter::makeColor("fillColor", "Fill Color", {255.0, 0.0, 0.0, 1.0}));
    addParameter(EffectParameter::makeFloat("tolerance", "Tolerance", 0.0, 100.0, 30.0));
    addParameter(EffectParameter::makeFloat("toleranceHue", "Hue Tolerance", 0.0, 100.0, 30.0));
}

std::unique_ptr<Effect> PaintBucketEffect::clone() const {
    auto e = std::make_unique<PaintBucketEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void PaintBucketEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    Vec2 fillPt = getVec2Param("fillPoint");
    Color fillColor = getColorParam("fillColor");
    float tolerance = getFloatParam("tolerance") * 2.55f;
    float hueTol = getFloatParam("toleranceHue") / 100.0f;

    int fx = std::clamp(static_cast<int>(fillPt.x * buffer.width), 0, buffer.width - 1);
    int fy = std::clamp(static_cast<int>(fillPt.y * buffer.height), 0, buffer.height - 1);
    const uint8_t* target = buffer.pixelAt(fx, fy);

    std::vector<bool> visited(buffer.width * buffer.height, false);
    std::vector<int> stack;
    stack.push_back(fy * buffer.width + fx);
    visited[fy * buffer.width + fx] = true;

    while (!stack.empty()) {
        int idx = stack.back(); stack.pop_back();
        int px = idx % buffer.width, py = idx / buffer.width;
        uint8_t* p = buffer.pixelAt(px, py);
        float dr = p[0] - target[0], dg = p[1] - target[1], db = p[2] - target[2];
        float dist = std::sqrt(dr * dr + dg * dg + db * db);
        if (dist <= tolerance) {
            p[0] = static_cast<uint8_t>(fillColor.r);
            p[1] = static_cast<uint8_t>(fillColor.g);
            p[2] = static_cast<uint8_t>(fillColor.b);
            p[3] = static_cast<uint8_t>(fillColor.a);
            int neighbors[] = {idx - 1, idx + 1, idx - buffer.width, idx + buffer.width};
            for (int n : neighbors) {
                if (n >= 0 && n < buffer.width * buffer.height && !visited[n]) {
                    visited[n] = true;
                    stack.push_back(n);
                }
            }
        }
    }
}

} // namespace FreeEffect
