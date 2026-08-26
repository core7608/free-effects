#include "transition_pack_plugin.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace FreeEffect {

#ifndef FREEEFFECT_BUILTIN_PLUGIN
extern "C" PluginInterface* createPlugin() {
    return new TransitionPackPlugin();
}

extern "C" void destroyPlugin(PluginInterface* plugin) {
    delete plugin;
}
#endif

TransitionPackPlugin::TransitionPackPlugin() {
    m_parameters.push_back({"transitionType", "Transition Type", "Type of transition", 0, 21, 0, 0, true, PluginParameter::Type::Enum});
    m_parameters.back().enumOptions = {
        "Fade", "Wipe Left", "Wipe Right", "Wipe Up", "Wipe Down",
        "Zoom In", "Zoom Out", "Slide Left", "Slide Right", "Slide Up", "Slide Down",
        "Dissolve", "Blur", "Glow", "Morph", "Clock Wipe", "Diamond Wipe",
        "Cross Zoom", "Push Left", "Push Right", "Luma Fade", "Radial Wipe"
    };
    m_parameters.push_back({"progress", "Progress", "Transition progress 0-1", 0.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"softness", "Softness", "Edge softness", 0.0, 100.0, 10.0, 10.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"reverse", "Reverse", "Reverse direction", 0.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Bool});
    m_parameters.push_back({"easing", "Easing", "Transition easing strength", 0.0, 1.0, 0.3, 0.3, true, PluginParameter::Type::Float});
}

TransitionPackPlugin::~TransitionPackPlugin() {
    shutdown();
}

PluginResult TransitionPackPlugin::initialize() {
    m_initialized = true;
    return PluginResult::ok();
}

PluginResult TransitionPackPlugin::shutdown() {
    m_initialized = false;
    return PluginResult::ok();
}

const char* TransitionPackPlugin::getName() const { return "Transition Pack"; }
const char* TransitionPackPlugin::getVersion() const { return "1.0.0"; }
const char* TransitionPackPlugin::getDescription() const { return "20+ professional transitions: fade, wipe, zoom, slide, dissolve, and more"; }
const char* TransitionPackPlugin::getAuthor() const { return "FreeEffect Team"; }
PluginAPIVersion TransitionPackPlugin::getAPIVersion() const { return PluginAPIVersion::v1_0; }
PluginType TransitionPackPlugin::getType() const { return PluginType::Transition; }

uint32_t TransitionPackPlugin::getParameterCount() const {
    return static_cast<uint32_t>(m_parameters.size());
}

PluginParameter TransitionPackPlugin::getParameter(uint32_t index) const {
    if (index < m_parameters.size()) {
        return m_parameters[index];
    }
    return {};
}

PluginResult TransitionPackPlugin::setParameter(const std::string& name, double value) {
    for (auto& param : m_parameters) {
        if (param.name == name) {
            param.currentValue = std::clamp(value, param.minValue, param.maxValue);
            return PluginResult::ok();
        }
    }
    return PluginResult::error("Unknown parameter: " + name);
}

double TransitionPackPlugin::getParameter(const std::string& name) const {
    for (const auto& param : m_parameters) {
        if (param.name == name) {
            return param.currentValue;
        }
    }
    return 0.0;
}

bool TransitionPackPlugin::needsProcessing() const { return true; }

PluginResult TransitionPackPlugin::process(float* buffer, int width, int height, int channels, double time) {
    if (!buffer || width <= 0 || height <= 0) return PluginResult::error("Invalid buffer");

    float progress = static_cast<float>(getParameter("progress"));
    float easing = static_cast<float>(getParameter("easing"));
    if (easing > 0.0f) {
        progress = progress * progress * (3.0f - 2.0f * progress);
    }
    if (static_cast<int>(getParameter("reverse")) != 0) {
        progress = 1.0f - progress;
    }

    int type = static_cast<int>(getParameter("transitionType"));
    switch (static_cast<TransitionPreset>(type)) {
        case TransitionPreset::Fade:
            applyFade(buffer, width, height, channels, progress);
            break;
        case TransitionPreset::WipeLeft:
            applyWipe(buffer, width, height, channels, progress, 0);
            break;
        case TransitionPreset::WipeRight:
            applyWipe(buffer, width, height, channels, progress, 1);
            break;
        case TransitionPreset::WipeUp:
            applyWipe(buffer, width, height, channels, progress, 2);
            break;
        case TransitionPreset::WipeDown:
            applyWipe(buffer, width, height, channels, progress, 3);
            break;
        case TransitionPreset::ZoomIn:
            applyZoom(buffer, width, height, channels, progress, true);
            break;
        case TransitionPreset::ZoomOut:
            applyZoom(buffer, width, height, channels, progress, false);
            break;
        case TransitionPreset::SlideLeft:
            applySlide(buffer, width, height, channels, progress, 0);
            break;
        case TransitionPreset::SlideRight:
            applySlide(buffer, width, height, channels, progress, 1);
            break;
        case TransitionPreset::SlideUp:
            applySlide(buffer, width, height, channels, progress, 2);
            break;
        case TransitionPreset::SlideDown:
            applySlide(buffer, width, height, channels, progress, 3);
            break;
        case TransitionPreset::Dissolve:
            applyDissolve(buffer, width, height, channels, progress);
            break;
        case TransitionPreset::Blur:
            applyBlur(buffer, width, height, channels, progress);
            break;
        case TransitionPreset::Glow:
            applyGlow(buffer, width, height, channels, progress);
            break;
        case TransitionPreset::Morph:
            applyMorph(buffer, width, height, channels, progress);
            break;
        case TransitionPreset::ClockWipe:
            applyClockWipe(buffer, width, height, channels, progress);
            break;
        case TransitionPreset::DiamondWipe:
            applyDiamondWipe(buffer, width, height, channels, progress);
            break;
        case TransitionPreset::CrossZoom:
            applyCrossZoom(buffer, width, height, channels, progress);
            break;
        case TransitionPreset::PushLeft:
            applyPush(buffer, width, height, channels, progress, 0);
            break;
        case TransitionPreset::PushRight:
            applyPush(buffer, width, height, channels, progress, 1);
            break;
        case TransitionPreset::LumaFade:
            applyLumaFade(buffer, width, height, channels, progress);
            break;
        case TransitionPreset::RadialWipe:
            applyRadialWipe(buffer, width, height, channels, progress);
            break;
    }

    return PluginResult::ok();
}

float TransitionPackPlugin::clampf(float v) const {
    return std::clamp(v, 0.0f, 1.0f);
}

void TransitionPackPlugin::applyFade(float* buffer, int width, int height, int channels, float progress) {
    for (int i = 0; i < width * height * channels; ++i) {
        buffer[i] *= progress;
    }
}

void TransitionPackPlugin::applyWipe(float* buffer, int width, int height, int channels, float progress, int direction) {
    float softness = static_cast<float>(getParameter("softness")) / 100.0f;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float pos = 0.0f;
            switch (direction) {
                case 0: pos = static_cast<float>(x) / width; break;
                case 1: pos = 1.0f - static_cast<float>(x) / width; break;
                case 2: pos = static_cast<float>(y) / height; break;
                case 3: pos = 1.0f - static_cast<float>(y) / height; break;
            }

            float edge = progress;
            float mask = std::clamp((pos - edge + softness) / (softness * 2.0f), 0.0f, 1.0f);

            int idx = (y * width + x) * channels;
            for (int c = 0; c < channels; ++c) {
                buffer[idx + c] *= mask;
            }
        }
    }
}

void TransitionPackPlugin::applyZoom(float* buffer, int width, int height, int channels, float progress, bool zoomIn) {
    std::vector<float> original(buffer, buffer + width * height * channels);
    float cx = width / 2.0f;
    float cy = height / 2.0f;
    float scale = zoomIn ? progress : (2.0f - progress);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int srcX = static_cast<int>((x - cx) / scale + cx);
            int srcY = static_cast<int>((y - cy) / scale + cy);

            int idx = (y * width + x) * channels;
            if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) {
                int srcIdx = (srcY * width + srcX) * channels;
                for (int c = 0; c < channels; ++c) {
                    buffer[idx + c] = original[srcIdx + c] * progress;
                }
            } else {
                for (int c = 0; c < channels; ++c) {
                    buffer[idx + c] = 0.0f;
                }
            }
        }
    }
}

void TransitionPackPlugin::applySlide(float* buffer, int width, int height, int channels, float progress, int direction) {
    std::vector<float> original(buffer, buffer + width * height * channels);
    int offset = static_cast<int>(progress * (direction < 2 ? width : height));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int srcX = x, srcY = y;
            switch (direction) {
                case 0: srcX = x + offset; break;
                case 1: srcX = x - offset; break;
                case 2: srcY = y + offset; break;
                case 3: srcY = y - offset; break;
            }

            int idx = (y * width + x) * channels;
            if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) {
                int srcIdx = (srcY * width + srcX) * channels;
                for (int c = 0; c < channels; ++c) {
                    buffer[idx + c] = original[srcIdx + c];
                }
            } else {
                for (int c = 0; c < channels; ++c) {
                    buffer[idx + c] = 0.0f;
                }
            }
        }
    }
}

void TransitionPackPlugin::applyDissolve(float* buffer, int width, int height, int channels, float progress) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int i = 0; i < width * height; ++i) {
        float threshold = dist(rng);
        float mask = (threshold < progress) ? 1.0f : 0.0f;
        for (int c = 0; c < channels; ++c) {
            buffer[i * channels + c] *= mask;
        }
    }
}

void TransitionPackPlugin::applyBlur(float* buffer, int width, int height, int channels, float progress) {
    int radius = static_cast<int>(progress * 15.0f);
    if (radius <= 0) return;
    applyBoxBlur(buffer, width, height, channels, radius);

    for (int i = 0; i < width * height * channels; ++i) {
        buffer[i] *= (1.0f - progress * 0.5f);
    }
}

void TransitionPackPlugin::applyGlow(float* buffer, int width, int height, int channels, float progress) {
    std::vector<float> original(buffer, buffer + width * height * channels);
    applyBoxBlur(buffer, width, height, channels, static_cast<int>(progress * 10.0f));

    for (int i = 0; i < width * height * channels; ++i) {
        buffer[i] = std::clamp(original[i] + buffer[i] * progress * 2.0f, 0.0f, 1.0f);
    }
}

void TransitionPackPlugin::applyClockWipe(float* buffer, int width, int height, int channels, float progress) {
    float cx = width / 2.0f;
    float cy = height / 2.0f;
    float angle = progress * 3.14159f * 2.0f;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = x - cx;
            float dy = y - cy;
            float a = std::atan2(dy, dx) + 3.14159f;
            float mask = (a <= angle) ? 1.0f : 0.0f;

            int idx = (y * width + x) * channels;
            for (int c = 0; c < channels; ++c) {
                buffer[idx + c] *= mask;
            }
        }
    }
}

void TransitionPackPlugin::applyDiamondWipe(float* buffer, int width, int height, int channels, float progress) {
    float cx = width / 2.0f;
    float cy = height / 2.0f;
    float size = (width + height) / 2.0f * progress;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dist = std::abs(x - cx) / cx + std::abs(y - cy) / cy;
            float mask = (dist <= progress * 2.0f) ? 1.0f : 0.0f;

            int idx = (y * width + x) * channels;
            for (int c = 0; c < channels; ++c) {
                buffer[idx + c] *= mask;
            }
        }
    }
}

void TransitionPackPlugin::applyCrossZoom(float* buffer, int width, int height, int channels, float progress) {
    float midProgress = 1.0f - std::abs(progress - 0.5f) * 2.0f;
    float scale = 1.0f + midProgress * 2.0f;

    std::vector<float> original(buffer, buffer + width * height * channels);
    float cx = width / 2.0f;
    float cy = height / 2.0f;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int srcX = static_cast<int>((x - cx) / scale + cx);
            int srcY = static_cast<int>((y - cy) / scale + cy);

            int idx = (y * width + x) * channels;
            if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) {
                int srcIdx = (srcY * width + srcX) * channels;
                for (int c = 0; c < channels; ++c) {
                    buffer[idx + c] = original[srcIdx + c] * progress;
                }
            } else {
                for (int c = 0; c < channels; ++c) {
                    buffer[idx + c] = 0.0f;
                }
            }
        }
    }
}

void TransitionPackPlugin::applyPush(float* buffer, int width, int height, int channels, float progress, int direction) {
    std::vector<float> original(buffer, buffer + width * height * channels);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int srcX = x, srcY = y;
            switch (direction) {
                case 0: srcX = x + static_cast<int>(progress * width); break;
                case 1: srcX = x - static_cast<int>(progress * width); break;
                case 2: srcY = y + static_cast<int>(progress * height); break;
                case 3: srcY = y - static_cast<int>(progress * height); break;
            }

            int idx = (y * width + x) * channels;
            if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) {
                int srcIdx = (srcY * width + srcX) * channels;
                for (int c = 0; c < channels; ++c) {
                    buffer[idx + c] = original[srcIdx + c];
                }
            } else {
                for (int c = 0; c < channels; ++c) {
                    buffer[idx + c] = 0.0f;
                }
            }
        }
    }
}

void TransitionPackPlugin::applyLumaFade(float* buffer, int width, int height, int channels, float progress) {
    for (int i = 0; i < width * height; ++i) {
        int idx = i * channels;
        float lum = (channels >= 3) ?
            (buffer[idx] * 0.2126f + buffer[idx + 1] * 0.7152f + buffer[idx + 2] * 0.0722f) :
            buffer[idx];
        float mask = std::clamp((lum - (1.0f - progress)) / 0.1f, 0.0f, 1.0f);
        for (int c = 0; c < channels; ++c) {
            buffer[idx + c] *= mask;
        }
    }
}

void TransitionPackPlugin::applyRadialWipe(float* buffer, int width, int height, int channels, float progress) {
    float cx = width / 2.0f;
    float cy = height / 2.0f;
    float maxR = std::sqrt(cx * cx + cy * cy);
    float radius = maxR * progress;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = x - cx;
            float dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float mask = (dist <= radius) ? 1.0f : 0.0f;

            int idx = (y * width + x) * channels;
            for (int c = 0; c < channels; ++c) {
                buffer[idx + c] *= mask;
            }
        }
    }
}

void TransitionPackPlugin::applyMorph(float* buffer, int width, int height, int channels, float progress) {
    for (int i = 0; i < width * height * channels; ++i) {
        float original = buffer[i];
        float target = 1.0f - original;
        buffer[i] = original + (target - original) * progress;
    }
}

void TransitionPackPlugin::applyBoxBlur(float* buffer, int width, int height, int channels, int radius) {
    if (radius <= 0) return;
    std::vector<float> original(buffer, buffer + width * height * channels);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channels; ++c) {
                float sum = 0.0f;
                int count = 0;
                for (int dy = -radius; dy <= radius; ++dy) {
                    for (int dx = -radius; dx <= radius; ++dx) {
                        int sx = x + dx;
                        int sy = y + dy;
                        if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
                            sum += original[(sy * width + sx) * channels + c];
                            count++;
                        }
                    }
                }
                buffer[(y * width + x) * channels + c] = (count > 0) ? sum / count : 0.0f;
            }
        }
    }
}

} // namespace FreeEffect
