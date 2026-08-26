#include "../effect_registry.h"
#include "equalize_effect.h"
#include <algorithm>
#include <numeric>
#include <vector>

namespace FreeEffect {

static EffectRegistrar<EqualizeEffect> s_reg("Equalize", "Color Correction");

EqualizeEffect::EqualizeEffect() {
    addParameter(EffectParameter::makeDropdown("equalizeBy", "Equalize By", {"RGB", "Brightness", "Hue"}, 1));
}

std::unique_ptr<Effect> EqualizeEffect::clone() const {
    auto e = std::make_unique<EqualizeEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void EqualizeEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    int mode = getDropdownParam("equalizeBy");
    int pixelCount = buffer.width * buffer.height;

    if (mode == 1 || mode == 2) {
        std::vector<float> lumas(pixelCount);
        std::vector<int> indices(pixelCount);
        for (int i = 0; i < pixelCount; i++) {
            const uint8_t* p = buffer.data.data() + i * 4;
            lumas[i] = 0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2];
            indices[i] = i;
        }
        std::sort(indices.begin(), indices.end(), [&](int a, int b) { return lumas[a] < lumas[b]; });

        std::vector<uint8_t> result(buffer.data.size());
        std::copy(buffer.data.begin(), buffer.data.end(), result.begin());
        for (int i = 0; i < pixelCount; i++) {
            float newVal = (static_cast<float>(i) / pixelCount) * 255.0f;
            uint8_t val = static_cast<uint8_t>(std::clamp(newVal, 0.0f, 255.0f));
            int idx = indices[i];
            result[idx * 4 + 0] = val;
            result[idx * 4 + 1] = val;
            result[idx * 4 + 2] = val;
        }
        buffer.data = result;
    } else {
        for (int ch = 0; ch < 3; ch++) {
            std::vector<int> hist(256, 0);
            for (int i = 0; i < pixelCount; i++) {
                hist[buffer.data[i * 4 + ch]]++;
            }
            std::vector<int> cdf(256, 0);
            cdf[0] = hist[0];
            for (int i = 1; i < 256; i++) cdf[i] = cdf[i - 1] + hist[i];
            int cdfMin = 0;
            for (int i = 0; i < 256; i++) { if (cdf[i] > 0) { cdfMin = cdf[i]; break; } }
            uint8_t lut[256];
            for (int i = 0; i < 256; i++) {
                lut[i] = static_cast<uint8_t>(std::clamp(
                    static_cast<float>(cdf[i] - cdfMin) / (pixelCount - cdfMin) * 255.0f, 0.0f, 255.0f));
            }
            for (int i = 0; i < pixelCount; i++) {
                buffer.data[i * 4 + ch] = lut[buffer.data[i * 4 + ch]];
            }
        }
    }
}

} // namespace FreeEffect
