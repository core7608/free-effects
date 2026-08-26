#include "text_animator_plugin.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace FreeEffect {

#ifndef FREEEFFECT_BUILTIN_PLUGIN
extern "C" PluginInterface* createPlugin() {
    return new TextAnimatorPlugin();
}

extern "C" void destroyPlugin(PluginInterface* plugin) {
    delete plugin;
}
#endif

TextAnimatorPlugin::TextAnimatorPlugin() {
    m_parameters.push_back({"preset", "Animation Preset", "Text animation preset", 0, 7, 0, 0, true, PluginParameter::Type::Enum});
    m_parameters.back().enumOptions = {"None", "Typewriter", "Fade In Per Char", "Bounce", "Scale Up", "Rotate In", "Blur In", "Wave"};
    m_parameters.push_back({"charCount", "Character Count", "Number of characters to animate", 1, 100, 20, 20, true, PluginParameter::Type::Int});
    m_parameters.push_back({"animDuration", "Duration", "Animation duration per character in seconds", 0.01, 5.0, 0.05, 0.05, true, PluginParameter::Type::Float});
    m_parameters.push_back({"delay", "Delay", "Delay between characters", 0.0, 1.0, 0.02, 0.02, true, PluginParameter::Type::Float});
    m_parameters.push_back({"fontSize", "Font Size", "Character size in pixels", 4.0, 200.0, 32.0, 32.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"textX", "Text X", "Text origin X position", 0.0, 1.0, 0.5, 0.5, true, PluginParameter::Type::Float});
    m_parameters.push_back({"textY", "Text Y", "Text origin Y position", 0.0, 1.0, 0.5, 0.5, true, PluginParameter::Type::Float});
    m_parameters.push_back({"spacing", "Spacing", "Character spacing", -20.0, 50.0, 5.0, 5.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"textR", "Text Red", "Text color red", 0.0, 1.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"textG", "Text Green", "Text color green", 0.0, 1.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"textB", "Text Blue", "Text color blue", 0.0, 1.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"bounceAmount", "Bounce Amount", "Bounce height", 0.0, 200.0, 50.0, 50.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"waveAmplitude", "Wave Amplitude", "Wave displacement amplitude", 0.0, 100.0, 20.0, 20.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"waveFrequency", "Wave Frequency", "Wave oscillation frequency", 0.1, 10.0, 2.0, 2.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"blurAmount", "Blur Amount", "Blur for blur-in effect", 0.0, 30.0, 15.0, 15.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"easing", "Easing", "Animation easing strength", 0.0, 1.0, 0.5, 0.5, true, PluginParameter::Type::Float});
}

TextAnimatorPlugin::~TextAnimatorPlugin() {
    shutdown();
}

PluginResult TextAnimatorPlugin::initialize() {
    m_initialized = true;
    return PluginResult::ok();
}

PluginResult TextAnimatorPlugin::shutdown() {
    m_initialized = false;
    return PluginResult::ok();
}

const char* TextAnimatorPlugin::getName() const { return "Text Animator"; }
const char* TextAnimatorPlugin::getVersion() const { return "1.0.0"; }
const char* TextAnimatorPlugin::getDescription() const { return "Advanced per-character text animation with presets"; }
const char* TextAnimatorPlugin::getAuthor() const { return "FreeEffect Team"; }
PluginAPIVersion TextAnimatorPlugin::getAPIVersion() const { return PluginAPIVersion::v1_0; }
PluginType TextAnimatorPlugin::getType() const { return PluginType::Effect; }

uint32_t TextAnimatorPlugin::getParameterCount() const {
    return static_cast<uint32_t>(m_parameters.size());
}

PluginParameter TextAnimatorPlugin::getParameter(uint32_t index) const {
    if (index < m_parameters.size()) {
        return m_parameters[index];
    }
    return {};
}

PluginResult TextAnimatorPlugin::setParameter(const std::string& name, double value) {
    for (auto& param : m_parameters) {
        if (param.name == name) {
            param.currentValue = std::clamp(value, param.minValue, param.maxValue);
            return PluginResult::ok();
        }
    }
    return PluginResult::error("Unknown parameter: " + name);
}

double TextAnimatorPlugin::getParameter(const std::string& name) const {
    for (const auto& param : m_parameters) {
        if (param.name == name) {
            return param.currentValue;
        }
    }
    return 0.0;
}

bool TextAnimatorPlugin::needsProcessing() const { return true; }

PluginResult TextAnimatorPlugin::process(float* buffer, int width, int height, int channels, double time) {
    if (!buffer || width <= 0 || height <= 0) return PluginResult::error("Invalid buffer");

    int preset = static_cast<int>(getParameter("preset"));
    switch (static_cast<TextAnimPreset>(preset)) {
        case TextAnimPreset::Typewriter:
            applyTypewriter(buffer, width, height, channels, time);
            break;
        case TextAnimPreset::FadeInPerChar:
            applyFadeInPerChar(buffer, width, height, channels, time);
            break;
        case TextAnimPreset::Bounce:
            applyBounce(buffer, width, height, channels, time);
            break;
        case TextAnimPreset::ScaleUp:
            applyScaleUp(buffer, width, height, channels, time);
            break;
        case TextAnimPreset::RotateIn:
            applyRotateIn(buffer, width, height, channels, time);
            break;
        case TextAnimPreset::BlurIn:
            applyBlurIn(buffer, width, height, channels, time);
            break;
        case TextAnimPreset::Wave:
            applyWave(buffer, width, height, channels, time);
            break;
        case TextAnimPreset::None:
        default:
            break;
    }

    return PluginResult::ok();
}

float TextAnimatorPlugin::clampf(float v) const {
    return std::clamp(v, 0.0f, 1.0f);
}

void TextAnimatorPlugin::renderCharacter(float* buffer, int width, int height, int channels, int cx, int cy, float size, float r, float g, float b, float a, float rotation) {
    int radius = static_cast<int>(size / 2.0f);
    float cosR = std::cos(rotation);
    float sinR = std::sin(rotation);

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            float rx = dx * cosR - dy * sinR;
            float ry = dx * sinR + dy * cosR;
            float dist = std::sqrt(rx * rx + ry * ry);
            if (dist > radius) continue;

            int sx = cx + dx;
            int sy = cy + dy;
            if (sx < 0 || sx >= width || sy < 0 || sy >= height) continue;

            float edgeFade = 1.0f - std::clamp((dist - radius + 2.0f) / 2.0f, 0.0f, 1.0f);
            float alpha = a * edgeFade;

            int idx = (sy * width + sx) * channels;
            if (channels >= 3) {
                buffer[idx] = std::clamp(buffer[idx] * (1.0f - alpha) + r * alpha, 0.0f, 1.0f);
                buffer[idx + 1] = std::clamp(buffer[idx + 1] * (1.0f - alpha) + g * alpha, 0.0f, 1.0f);
                buffer[idx + 2] = std::clamp(buffer[idx + 2] * (1.0f - alpha) + b * alpha, 0.0f, 1.0f);
            }
            if (channels >= 4) {
                buffer[idx + 3] = std::clamp(buffer[idx + 3] * (1.0f - alpha) + alpha, 0.0f, 1.0f);
            }
        }
    }
}

void TextAnimatorPlugin::applyTypewriter(float* buffer, int width, int height, int channels, double time) {
    int count = static_cast<int>(getParameter("charCount"));
    float dur = static_cast<float>(getParameter("animDuration"));
    float delay = static_cast<float>(getParameter("delay"));
    float fontSize = static_cast<float>(getParameter("fontSize"));
    float originX = static_cast<float>(getParameter("textX")) * width;
    float originY = static_cast<float>(getParameter("textY")) * height;
    float spacing = static_cast<float>(getParameter("spacing"));
    float r = static_cast<float>(getParameter("textR"));
    float g = static_cast<float>(getParameter("textG"));
    float b = static_cast<float>(getParameter("textB"));
    float t = static_cast<float>(time);

    for (int i = 0; i < count; ++i) {
        float charTime = t - i * delay;
        if (charTime < 0.0f) continue;
        float progress = std::clamp(charTime / dur, 0.0f, 1.0f);
        if (progress < 1.0f) continue;

        float cx = originX + i * (fontSize + spacing);
        float cy = originY;
        renderCharacter(buffer, width, height, channels,
                        static_cast<int>(cx), static_cast<int>(cy), fontSize,
                        r, g, b, 1.0f, 0.0f);
    }
}

void TextAnimatorPlugin::applyFadeInPerChar(float* buffer, int width, int height, int channels, double time) {
    int count = static_cast<int>(getParameter("charCount"));
    float dur = static_cast<float>(getParameter("animDuration"));
    float delay = static_cast<float>(getParameter("delay"));
    float fontSize = static_cast<float>(getParameter("fontSize"));
    float originX = static_cast<float>(getParameter("textX")) * width;
    float originY = static_cast<float>(getParameter("textY")) * height;
    float spacing = static_cast<float>(getParameter("spacing"));
    float r = static_cast<float>(getParameter("textR"));
    float g = static_cast<float>(getParameter("textG"));
    float b = static_cast<float>(getParameter("textB"));
    float t = static_cast<float>(time);

    for (int i = 0; i < count; ++i) {
        float charTime = t - i * delay;
        if (charTime < 0.0f) continue;
        float alpha = std::clamp(charTime / dur, 0.0f, 1.0f);

        float cx = originX + i * (fontSize + spacing);
        float cy = originY;
        renderCharacter(buffer, width, height, channels,
                        static_cast<int>(cx), static_cast<int>(cy), fontSize,
                        r, g, b, alpha, 0.0f);
    }
}

void TextAnimatorPlugin::applyBounce(float* buffer, int width, int height, int channels, double time) {
    int count = static_cast<int>(getParameter("charCount"));
    float dur = static_cast<float>(getParameter("animDuration"));
    float delay = static_cast<float>(getParameter("delay"));
    float fontSize = static_cast<float>(getParameter("fontSize"));
    float originX = static_cast<float>(getParameter("textX")) * width;
    float originY = static_cast<float>(getParameter("textY")) * height;
    float spacing = static_cast<float>(getParameter("spacing"));
    float bounceAmt = static_cast<float>(getParameter("bounceAmount"));
    float r = static_cast<float>(getParameter("textR"));
    float g = static_cast<float>(getParameter("textG"));
    float b = static_cast<float>(getParameter("textB"));
    float t = static_cast<float>(time);

    for (int i = 0; i < count; ++i) {
        float charTime = t - i * delay;
        if (charTime < 0.0f) continue;
        float progress = std::clamp(charTime / dur, 0.0f, 1.0f);

        float bounceY = 0.0f;
        if (progress < 1.0f) {
            bounceY = -bounceAmt * std::sin(progress * 3.14159f * 3.0f) * (1.0f - progress);
        }

        float alpha = std::clamp(progress * 3.0f, 0.0f, 1.0f);
        float cx = originX + i * (fontSize + spacing);
        float cy = originY + bounceY;
        renderCharacter(buffer, width, height, channels,
                        static_cast<int>(cx), static_cast<int>(cy), fontSize,
                        r, g, b, alpha, 0.0f);
    }
}

void TextAnimatorPlugin::applyScaleUp(float* buffer, int width, int height, int channels, double time) {
    int count = static_cast<int>(getParameter("charCount"));
    float dur = static_cast<float>(getParameter("animDuration"));
    float delay = static_cast<float>(getParameter("delay"));
    float fontSize = static_cast<float>(getParameter("fontSize"));
    float originX = static_cast<float>(getParameter("textX")) * width;
    float originY = static_cast<float>(getParameter("textY")) * height;
    float spacing = static_cast<float>(getParameter("spacing"));
    float r = static_cast<float>(getParameter("textR"));
    float g = static_cast<float>(getParameter("textG"));
    float b = static_cast<float>(getParameter("textB"));
    float t = static_cast<float>(time);

    for (int i = 0; i < count; ++i) {
        float charTime = t - i * delay;
        if (charTime < 0.0f) continue;
        float progress = std::clamp(charTime / dur, 0.0f, 1.0f);

        float scale = progress * progress * (3.0f - 2.0f * progress);
        float alpha = std::clamp(progress * 2.0f, 0.0f, 1.0f);
        float size = fontSize * scale;

        float cx = originX + i * (fontSize + spacing);
        float cy = originY;
        renderCharacter(buffer, width, height, channels,
                        static_cast<int>(cx), static_cast<int>(cy), size,
                        r, g, b, alpha, 0.0f);
    }
}

void TextAnimatorPlugin::applyRotateIn(float* buffer, int width, int height, int channels, double time) {
    int count = static_cast<int>(getParameter("charCount"));
    float dur = static_cast<float>(getParameter("animDuration"));
    float delay = static_cast<float>(getParameter("delay"));
    float fontSize = static_cast<float>(getParameter("fontSize"));
    float originX = static_cast<float>(getParameter("textX")) * width;
    float originY = static_cast<float>(getParameter("textY")) * height;
    float spacing = static_cast<float>(getParameter("spacing"));
    float r = static_cast<float>(getParameter("textR"));
    float g = static_cast<float>(getParameter("textG"));
    float b = static_cast<float>(getParameter("textB"));
    float t = static_cast<float>(time);

    for (int i = 0; i < count; ++i) {
        float charTime = t - i * delay;
        if (charTime < 0.0f) continue;
        float progress = std::clamp(charTime / dur, 0.0f, 1.0f);

        float eased = progress * progress * (3.0f - 2.0f * progress);
        float rotation = (1.0f - eased) * 3.14159f * 0.5f;
        float alpha = std::clamp(progress * 2.0f, 0.0f, 1.0f);

        float cx = originX + i * (fontSize + spacing);
        float cy = originY;
        renderCharacter(buffer, width, height, channels,
                        static_cast<int>(cx), static_cast<int>(cy), fontSize,
                        r, g, b, alpha, rotation);
    }
}

void TextAnimatorPlugin::applyBlurIn(float* buffer, int width, int height, int channels, double time) {
    int count = static_cast<int>(getParameter("charCount"));
    float dur = static_cast<float>(getParameter("animDuration"));
    float delay = static_cast<float>(getParameter("delay"));
    float fontSize = static_cast<float>(getParameter("fontSize"));
    float originX = static_cast<float>(getParameter("textX")) * width;
    float originY = static_cast<float>(getParameter("textY")) * height;
    float spacing = static_cast<float>(getParameter("spacing"));
    float blurAmt = static_cast<float>(getParameter("blurAmount"));
    float r = static_cast<float>(getParameter("textR"));
    float g = static_cast<float>(getParameter("textG"));
    float b = static_cast<float>(getParameter("textB"));
    float t = static_cast<float>(time);

    for (int i = 0; i < count; ++i) {
        float charTime = t - i * delay;
        if (charTime < 0.0f) continue;
        float progress = std::clamp(charTime / dur, 0.0f, 1.0f);

        float blur = (1.0f - progress) * blurAmt;
        float alpha = std::clamp(progress * 2.0f, 0.0f, 1.0f);
        float size = fontSize + blur * 2.0f;

        float cx = originX + i * (fontSize + spacing);
        float cy = originY;
        renderCharacter(buffer, width, height, channels,
                        static_cast<int>(cx), static_cast<int>(cy), size,
                        r * alpha, g * alpha, b * alpha, alpha * alpha, 0.0f);
    }
}

void TextAnimatorPlugin::applyWave(float* buffer, int width, int height, int channels, double time) {
    int count = static_cast<int>(getParameter("charCount"));
    float delay = static_cast<float>(getParameter("delay"));
    float fontSize = static_cast<float>(getParameter("fontSize"));
    float originX = static_cast<float>(getParameter("textX")) * width;
    float originY = static_cast<float>(getParameter("textY")) * height;
    float spacing = static_cast<float>(getParameter("spacing"));
    float waveAmp = static_cast<float>(getParameter("waveAmplitude"));
    float waveFreq = static_cast<float>(getParameter("waveFrequency"));
    float r = static_cast<float>(getParameter("textR"));
    float g = static_cast<float>(getParameter("textG"));
    float b = static_cast<float>(getParameter("textB"));
    float t = static_cast<float>(time);

    for (int i = 0; i < count; ++i) {
        float waveY = std::sin(t * waveFreq * 3.14159f * 2.0f + i * 0.5f) * waveAmp;
        float alpha = std::clamp(1.0f - std::abs(waveY) / waveAmp * 0.3f, 0.3f, 1.0f);

        float cx = originX + i * (fontSize + spacing);
        float cy = originY + waveY;
        renderCharacter(buffer, width, height, channels,
                        static_cast<int>(cx), static_cast<int>(cy), fontSize,
                        r, g, b, alpha, 0.0f);
    }
}

} // namespace FreeEffect
