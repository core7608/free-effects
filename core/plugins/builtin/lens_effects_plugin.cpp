#include "lens_effects_plugin.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace FreeEffect {

#ifndef FREEEFFECT_BUILTIN_PLUGIN
extern "C" PluginInterface* createPlugin() {
    return new LensEffectsPlugin();
}

extern "C" void destroyPlugin(PluginInterface* plugin) {
    delete plugin;
}
#endif

LensEffectsPlugin::LensEffectsPlugin() {
    m_parameters.push_back({"distortion", "Distortion", "Barrel/pincushion distortion", -2.0, 2.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"chromaticAberration", "Chromatic Aberration", "Color fringing amount", 0.0, 10.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"vignetteIntensity", "Vignette Intensity", "Darkening at edges", 0.0, 2.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"vignetteRadius", "Vignette Radius", "Size of vignette center", 0.1, 1.0, 0.5, 0.5, true, PluginParameter::Type::Float});
    m_parameters.push_back({"fisheye", "Fisheye", "Fisheye lens effect", 0.0, 2.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"lensFlare", "Lens Flare", "Lens flare brightness", 0.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"flareX", "Flare X", "Flare horizontal position", -1.0, 2.0, 0.5, 0.5, true, PluginParameter::Type::Float});
    m_parameters.push_back({"flareY", "Flare Y", "Flare vertical position", -1.0, 2.0, 0.3, 0.3, true, PluginParameter::Type::Float});
    m_parameters.push_back({"anamorphic", "Anamorphic", "Anamorphic squeeze", 0.5, 2.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"softFocus", "Soft Focus", "Soft focus glow amount", 0.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});
}

LensEffectsPlugin::~LensEffectsPlugin() {
    shutdown();
}

PluginResult LensEffectsPlugin::initialize() {
    m_initialized = true;
    return PluginResult::ok();
}

PluginResult LensEffectsPlugin::shutdown() {
    m_initialized = false;
    return PluginResult::ok();
}

const char* LensEffectsPlugin::getName() const { return "Lens Effects"; }
const char* LensEffectsPlugin::getVersion() const { return "1.0.0"; }
const char* LensEffectsPlugin::getDescription() const { return "Realistic lens simulation: distortion, chromatic aberration, vignette"; }
const char* LensEffectsPlugin::getAuthor() const { return "FreeEffect Team"; }
PluginAPIVersion LensEffectsPlugin::getAPIVersion() const { return PluginAPIVersion::v1_0; }
PluginType LensEffectsPlugin::getType() const { return PluginType::Effect; }

uint32_t LensEffectsPlugin::getParameterCount() const {
    return static_cast<uint32_t>(m_parameters.size());
}

PluginParameter LensEffectsPlugin::getParameter(uint32_t index) const {
    if (index < m_parameters.size()) {
        return m_parameters[index];
    }
    return {};
}

PluginResult LensEffectsPlugin::setParameter(const std::string& name, double value) {
    for (auto& param : m_parameters) {
        if (param.name == name) {
            param.currentValue = std::clamp(value, param.minValue, param.maxValue);
            return PluginResult::ok();
        }
    }
    return PluginResult::error("Unknown parameter: " + name);
}

double LensEffectsPlugin::getParameter(const std::string& name) const {
    for (const auto& param : m_parameters) {
        if (param.name == name) {
            return param.currentValue;
        }
    }
    return 0.0;
}

bool LensEffectsPlugin::needsProcessing() const { return true; }

PluginResult LensEffectsPlugin::process(float* buffer, int width, int height, int channels, double time) {
    if (!buffer || width <= 0 || height <= 0 || channels < 3) {
        return PluginResult::error("Invalid buffer");
    }

    float distortion = static_cast<float>(getParameter("distortion"));
    float ca = static_cast<float>(getParameter("chromaticAberration"));
    float vignetteIntensity = static_cast<float>(getParameter("vignetteIntensity"));
    float fisheye = static_cast<float>(getParameter("fisheye"));
    float anamorphic = static_cast<float>(getParameter("anamorphic"));

    if (std::abs(distortion) > 0.001f) {
        applyBarrelDistortion(buffer, width, height, channels);
    }

    if (std::abs(fisheye) > 0.001f) {
        applyFisheye(buffer, width, height, channels);
    }

    if (ca > 0.001f) {
        applyChromaticAberration(buffer, width, height, channels);
    }

    if (vignetteIntensity > 0.001f) {
        applyVignette(buffer, width, height, channels);
    }

    if (std::abs(anamorphic - 1.0f) > 0.001f) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int srcX = static_cast<int>((x / static_cast<float>(width) - 0.5f) * anamorphic * width + width / 2.0f);
                int srcY = y;
                if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) {
                    int dstIdx = (y * width + x) * channels;
                    int srcIdx = (srcY * width + srcX) * channels;
                    for (int c = 0; c < channels; ++c) {
                        buffer[dstIdx + c] = buffer[srcIdx + c];
                    }
                }
            }
        }
    }

    float softFocus = static_cast<float>(getParameter("softFocus"));
    if (softFocus > 0.001f) {
        std::vector<float> original(buffer, buffer + width * height * channels);
        for (int y = 1; y < height - 1; ++y) {
            for (int x = 1; x < width - 1; ++x) {
                for (int c = 0; c < channels; ++c) {
                    int idx = (y * width + x) * channels + c;
                    float blur = (original[idx] * 4.0f +
                                  original[((y - 1) * width + x) * channels + c] +
                                  original[((y + 1) * width + x) * channels + c] +
                                  original[(y * width + x - 1) * channels + c] +
                                  original[(y * width + x + 1) * channels + c]) / 8.0f;
                    buffer[idx] = buffer[idx] * (1.0f - softFocus) + blur * softFocus;
                }
            }
        }
    }

    return PluginResult::ok();
}

float LensEffectsPlugin::clampf(float v) const {
    return std::clamp(v, 0.0f, 1.0f);
}

void LensEffectsPlugin::applyDistortion(float* buffer, int width, int height, int channels) {
    float strength = static_cast<float>(getParameter("distortion"));
    std::vector<float> original(buffer, buffer + width * height * channels);

    float cx = width / 2.0f;
    float cy = height / 2.0f;
    float maxR = std::sqrt(cx * cx + cy * cy);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = (x - cx) / maxR;
            float dy = (y - cy) / maxR;
            float r2 = dx * dx + dy * dy;
            float distortionFactor = 1.0f + strength * r2;

            int srcX = static_cast<int>(cx + dx * distortionFactor * maxR);
            int srcY = static_cast<int>(cy + dy * distortionFactor * maxR);

            if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) {
                int dstIdx = (y * width + x) * channels;
                int srcIdx = (srcY * width + srcX) * channels;
                for (int c = 0; c < channels; ++c) {
                    buffer[dstIdx + c] = original[srcIdx + c];
                }
            }
        }
    }
}

void LensEffectsPlugin::applyChromaticAberration(float* buffer, int width, int height, int channels) {
    if (channels < 3) return;

    float ca = static_cast<float>(getParameter("chromaticAberration"));
    std::vector<float> original(buffer, buffer + width * height * channels);
    float cx = width / 2.0f;
    float cy = height / 2.0f;
    float maxR = std::sqrt(cx * cx + cy * cy);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = (x - cx) / maxR;
            float dy = (y - cy) / maxR;
            float r = std::sqrt(dx * dx + dy * dy);

            int dstIdx = (y * width + x) * channels;

            float redShift = ca * r * 0.01f;
            int redX = static_cast<int>(x + dx * redShift * maxR);
            int redY = static_cast<int>(y + dy * redShift * maxR);
            if (redX >= 0 && redX < width && redY >= 0 && redY < height) {
                int srcIdx = (redY * width + redX) * channels;
                buffer[dstIdx] = original[srcIdx];
            }

            buffer[dstIdx + 1] = original[dstIdx + 1];

            float blueShift = -ca * r * 0.01f;
            int blueX = static_cast<int>(x + dx * blueShift * maxR);
            int blueY = static_cast<int>(y + dy * blueShift * maxR);
            if (blueX >= 0 && blueX < width && blueY >= 0 && blueY < height) {
                int srcIdx = (blueY * width + blueX) * channels;
                buffer[dstIdx + 2] = original[srcIdx + 2];
            }
        }
    }
}

void LensEffectsPlugin::applyVignette(float* buffer, int width, int height, int channels) {
    float intensity = static_cast<float>(getParameter("vignetteIntensity"));
    float radius = static_cast<float>(getParameter("vignetteRadius"));

    float cx = width / 2.0f;
    float cy = height / 2.0f;
    float maxR = std::sqrt(cx * cx + cy * cy) * radius;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = x - cx;
            float dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float vignette = 1.0f - std::clamp((dist - maxR * 0.5f) / (maxR * 0.5f), 0.0f, 1.0f) * intensity;

            int idx = (y * width + x) * channels;
            for (int c = 0; c < channels; ++c) {
                buffer[idx + c] *= vignette;
            }
        }
    }
}

void LensEffectsPlugin::applyFisheye(float* buffer, int width, int height, int channels) {
    float strength = static_cast<float>(getParameter("fisheye"));
    std::vector<float> original(buffer, buffer + width * height * channels);
    float cx = width / 2.0f;
    float cy = height / 2.0f;
    float maxR = std::sqrt(cx * cx + cy * cy);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = (x - cx) / maxR;
            float dy = (y - cy) / maxR;
            float r = std::sqrt(dx * dx + dy * dy);

            if (r < 1.0f) {
                float theta = std::atan2(dy, dx);
                float newR = std::pow(r, 1.0f + strength);
                int srcX = static_cast<int>(cx + std::cos(theta) * newR * maxR);
                int srcY = static_cast<int>(cy + std::sin(theta) * newR * maxR);

                if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) {
                    int dstIdx = (y * width + x) * channels;
                    int srcIdx = (srcY * width + srcX) * channels;
                    for (int c = 0; c < channels; ++c) {
                        buffer[dstIdx + c] = original[srcIdx + c];
                    }
                }
            }
        }
    }
}

void LensEffectsPlugin::applyBarrelDistortion(float* buffer, int width, int height, int channels) {
    applyDistortion(buffer, width, height, channels);
}

} // namespace FreeEffect
