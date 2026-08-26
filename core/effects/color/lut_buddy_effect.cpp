#include "../effect_registry.h"
#include "lut_buddy_effect.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

namespace FreeEffect {

static EffectRegistrar<LutBuddyEffect> s_reg("LUT Buddy", "Color Correction");

LutBuddyEffect::LutBuddyEffect() {
    addParameter(EffectParameter::makeString("lutFile", "LUT File", ""));
    addParameter(EffectParameter::makeFloat("intensity", "Intensity", 0.0, 100.0, 100.0));
    addParameter(EffectParameter::makeDropdown("lutType", "LUT Type", {"1D (64)", "1D (256)", "3D (17x17x17)", "3D (33x33x33)"}, 2));
}

std::unique_ptr<Effect> LutBuddyEffect::clone() const {
    auto e = std::make_unique<LutBuddyEffect>();
    e->setEnabled(m_enabled);
    e->setOrder(m_order);
    return e;
}

void LutBuddyEffect::render(PixelBuffer& buffer, double time) {
    (void)time;
    float intensity = getFloatParam("intensity") / 100.0f;
    if (intensity <= 0) return;

    int lutType = getDropdownParam("lutType");
    std::string lutFile = getParameter("lutFile") ? getParameter("lutFile")->stringValue : std::string();

    std::vector<float> lutR, lutG, lutB;
    bool loaded = false;

    if (!lutFile.empty()) {
        std::ifstream file(lutFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;
                if (line.find("TITLE") != std::string::npos ||
                    line.find("LUT_3D_SIZE") != std::string::npos ||
                    line.find("DOMAIN_MIN") != std::string::npos ||
                    line.find("DOMAIN_MAX") != std::string::npos) continue;
                std::istringstream iss(line);
                float r, g, b;
                if (iss >> r >> g >> b) {
                    lutR.push_back(r);
                    lutG.push_back(g);
                    lutB.push_back(b);
                }
            }
            loaded = !lutR.empty();
        }
    }

    if (!loaded) {
        for (int i = 0; i < 256; i++) {
            float v = i / 255.0f;
            lutR.push_back(std::clamp(v * 1.1f, 0.0f, 1.0f));
            lutG.push_back(std::clamp(v * 1.0f, 0.0f, 1.0f));
            lutB.push_back(std::clamp(v * 0.9f, 0.0f, 1.0f));
        }
        loaded = true;
    }

    int lutSize = static_cast<int>(lutR.size());
    if (lutSize == 0) return;

    auto applyLUT = [&](float v, const std::vector<float>& lut) -> float {
        float idx = v * (lutSize - 1);
        int i0 = std::clamp(static_cast<int>(idx), 0, lutSize - 2);
        float frac = idx - i0;
        return lut[i0] * (1.0f - frac) + lut[i0 + 1] * frac;
    };

    for (int y = 0; y < buffer.height; y++) {
        for (int x = 0; x < buffer.width; x++) {
            uint8_t* p = buffer.pixelAt(x, y);
            float r = p[0] / 255.0f, g = p[1] / 255.0f, b = p[2] / 255.0f;
            float nr = applyLUT(r, lutR);
            float ng = applyLUT(g, lutG);
            float nb = applyLUT(b, lutB);
            p[0] = static_cast<uint8_t>(std::clamp((r + (nr - r) * intensity) * 255.0f, 0.0f, 255.0f));
            p[1] = static_cast<uint8_t>(std::clamp((g + (ng - g) * intensity) * 255.0f, 0.0f, 255.0f));
            p[2] = static_cast<uint8_t>(std::clamp((b + (nb - b) * intensity) * 255.0f, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
