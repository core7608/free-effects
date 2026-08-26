#include "../effect_registry.h"
#include "remove_grain_effect.h"
#include <algorithm>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<RemoveGrainEffect> s_reg("Remove Grain", "Noise & Grain");

RemoveGrainEffect::RemoveGrainEffect() {
    addParameter(EffectParameter::makeDropdown("viewingMode", "Viewing Mode",
        {"Preview", "Noise Sampling", "Blurring"}, 2));
    addParameter(EffectParameter::makeInt("temporalSamples", "Temporal Samples", 1, 30, 1));
    addParameter(EffectParameter::makeInt("previewRegion", "Preview Region Size", 1, 500, 100));
}

std::unique_ptr<Effect> RemoveGrainEffect::clone() const {
    auto e = std::make_unique<RemoveGrainEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void RemoveGrainEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int viewingMode = getDropdownParam("viewingMode");

    if (viewingMode == 0) return; // Preview mode, no change

    int ksize = 3;
    if (viewingMode == 2) ksize = 5;

    PixelBuffer tmp;
    tmp.resize(buffer.width, buffer.height);

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            int r = 0, g = 0, b = 0, a = 0, count = 0;
            for (int dy = -ksize; dy <= ksize; dy++) {
                for (int dx = -ksize; dx <= ksize; dx++) {
                    int sx = std::clamp(x + dx, 0, buffer.width - 1);
                    int sy = std::clamp(y + dy, 0, buffer.height - 1);
                    const uint8_t* p = buffer.pixelAt(sx, sy);
                    r += p[0]; g += p[1]; b += p[2]; a += p[3];
                    count++;
                }
            }
            uint8_t* dst = tmp.pixelAt(x, y);
            dst[0] = static_cast<uint8_t>(r / count);
            dst[1] = static_cast<uint8_t>(g / count);
            dst[2] = static_cast<uint8_t>(b / count);
            dst[3] = static_cast<uint8_t>(a / count);
        }
    }
    buffer.data = tmp.data;
}

} // namespace FreeEffect
