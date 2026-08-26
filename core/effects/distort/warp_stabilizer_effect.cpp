#include "../effect_registry.h"
#include "warp_stabilizer_effect.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

static EffectRegistrar<WarpStabilizerEffect> s_reg("Warp Stabilizer", "Distort");

WarpStabilizerEffect::WarpStabilizerEffect() {
    addParameter(EffectParameter::makeFloat("smoothness", "Smoothness", 0.0, 100.0, 50.0));
    addParameter(EffectParameter::makeFloat("stabilizationAmount", "Stabilization Amount", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeDropdown("stabilizeMethod", "Method", {"Position", "Position, Scale, Rotation", "Perspective", "Subspace Warp"}, 1));
    addParameter(EffectParameter::makeBool("borderCrop", "Crop Less, Smoother Motion", false));
}

std::unique_ptr<Effect> WarpStabilizerEffect::clone() const {
    auto e = std::make_unique<WarpStabilizerEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void WarpStabilizerEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float smoothness = getFloatParam("smoothness") / 100.0f;
    float amount = getFloatParam("stabilizationAmount") / 100.0f;
    if (amount <= 0) return;

    float t = static_cast<float>(time);
    float offsetX = std::sin(t * 0.5f) * smoothness * 10.0f * amount;
    float offsetY = std::cos(t * 0.3f) * smoothness * 8.0f * amount;
    float scale = 1.0f + smoothness * 0.05f * amount;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);
    std::copy(buffer.data.begin(), buffer.data.end(), tmp.data.begin());

    float cx = buffer.width / 2.0f, cy = buffer.height / 2.0f;
    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            float srcX = (x - cx - offsetX) / scale + cx;
            float srcY = (y - cy - offsetY) / scale + cy;
            int sx = std::clamp(static_cast<int>(srcX + 0.5f), 0, buffer.width - 1);
            int sy = std::clamp(static_cast<int>(srcY + 0.5f), 0, buffer.height - 1);
            const uint8_t* p = tmp.pixelAt(sx, sy);
            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = p[0]; dst[1] = p[1]; dst[2] = p[2]; dst[3] = p[3];
        }
    }
}

} // namespace FreeEffect
