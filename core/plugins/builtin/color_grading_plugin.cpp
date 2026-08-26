#include "color_grading_plugin.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace FreeEffect {

#ifndef FREEEFFECT_BUILTIN_PLUGIN
extern "C" PluginInterface* createPlugin() {
    return new ColorGradingPlugin();
}

extern "C" void destroyPlugin(PluginInterface* plugin) {
    delete plugin;
}
#endif

ColorGradingPlugin::ColorGradingPlugin() {
    m_parameters.push_back({"lift", "Lift", "Black level offset", -1.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"gamma", "Gamma", "Midtone curve", 0.1, 5.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"gain", "Gain", "White level multiplier", 0.0, 3.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"saturation", "Saturation", "Color saturation", 0.0, 3.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"temperature", "Temperature", "Color temperature", -1.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"tint", "Tint", "Green-magenta tint", -1.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"contrast", "Contrast", "Image contrast", 0.0, 2.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"brightness", "Brightness", "Image brightness", -1.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"highlights", "Highlights", "Highlight recovery", 0.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"shadows", "Shadows", "Shadow recovery", 0.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});

    m_parameters.push_back({"liftR", "Lift Red", "Red lift", -1.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"liftG", "Lift Green", "Green lift", -1.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"liftB", "Lift Blue", "Blue lift", -1.0, 1.0, 0.0, 0.0, true, PluginParameter::Type::Float});

    m_parameters.push_back({"gammaR", "Gamma Red", "Red gamma", 0.1, 5.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"gammaG", "Gamma Green", "Green gamma", 0.1, 5.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"gammaB", "Gamma Blue", "Blue gamma", 0.1, 5.0, 1.0, 1.0, true, PluginParameter::Type::Float});

    m_parameters.push_back({"gainR", "Gain Red", "Red gain", 0.0, 3.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"gainG", "Gain Green", "Green gain", 0.0, 3.0, 1.0, 1.0, true, PluginParameter::Type::Float});
    m_parameters.push_back({"gainB", "Gain Blue", "Blue gain", 0.0, 3.0, 1.0, 1.0, true, PluginParameter::Type::Float});
}

ColorGradingPlugin::~ColorGradingPlugin() {
    shutdown();
}

PluginResult ColorGradingPlugin::initialize() {
    m_initialized = true;
    return PluginResult::ok();
}

PluginResult ColorGradingPlugin::shutdown() {
    m_initialized = false;
    return PluginResult::ok();
}

const char* ColorGradingPlugin::getName() const { return "Color Grading"; }
const char* ColorGradingPlugin::getVersion() const { return "1.0.0"; }
const char* ColorGradingPlugin::getDescription() const { return "Advanced color grading with lift/gamma/gain wheels"; }
const char* ColorGradingPlugin::getAuthor() const { return "FreeEffect Team"; }
PluginAPIVersion ColorGradingPlugin::getAPIVersion() const { return PluginAPIVersion::v1_0; }
PluginType ColorGradingPlugin::getType() const { return PluginType::Effect; }

uint32_t ColorGradingPlugin::getParameterCount() const {
    return static_cast<uint32_t>(m_parameters.size());
}

PluginParameter ColorGradingPlugin::getParameter(uint32_t index) const {
    if (index < m_parameters.size()) {
        return m_parameters[index];
    }
    return {};
}

PluginResult ColorGradingPlugin::setParameter(const std::string& name, double value) {
    for (auto& param : m_parameters) {
        if (param.name == name) {
            param.currentValue = std::clamp(value, param.minValue, param.maxValue);
            return PluginResult::ok();
        }
    }
    return PluginResult::error("Unknown parameter: " + name);
}

double ColorGradingPlugin::getParameter(const std::string& name) const {
    for (const auto& param : m_parameters) {
        if (param.name == name) {
            return param.currentValue;
        }
    }
    return 0.0;
}

bool ColorGradingPlugin::needsProcessing() const { return true; }

PluginResult ColorGradingPlugin::process(float* buffer, int width, int height, int channels, double time) {
    if (!buffer || width <= 0 || height <= 0) {
        return PluginResult::error("Invalid buffer");
    }

    for (int i = 0; i < width * height; ++i) {
        int offset = i * channels;
        if (channels >= 3) {
            float r = buffer[offset] / 255.0f;
            float g = buffer[offset + 1] / 255.0f;
            float b = buffer[offset + 2] / 255.0f;

            applyLift(r, g, b);
            applyGamma(r, g, b);
            applyGain(r, g, b);
            applySaturation(r, g, b);
            applyTemperature(r, g, b);
            applyContrast(r, g, b);
            applyHighlights(r, g, b);
            applyShadows(r, g, b);

            float brightness = static_cast<float>(getParameter("brightness"));
            r += brightness;
            g += brightness;
            b += brightness;

            float tint = static_cast<float>(getParameter("tint"));
            g += tint * 0.2f;

            buffer[offset] = clampf(r) * 255.0f;
            buffer[offset + 1] = clampf(g) * 255.0f;
            buffer[offset + 2] = clampf(b) * 255.0f;
        }
    }

    return PluginResult::ok();
}

float ColorGradingPlugin::clampf(float v) const {
    return std::clamp(v, 0.0f, 1.0f);
}

float ColorGradingPlugin::luminance(float r, float g, float b) const {
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

void ColorGradingPlugin::applyLift(float& r, float& g, float& b) const {
    float lift = static_cast<float>(getParameter("lift"));
    float liftR = static_cast<float>(getParameter("liftR"));
    float liftG = static_cast<float>(getParameter("liftG"));
    float liftB = static_cast<float>(getParameter("liftB"));

    float lum = luminance(r, g, b);
    float liftAmount = (1.0f - std::pow(1.0f - lum, 2.0f)) * lift;

    r += liftAmount + liftR * 0.5f;
    g += liftAmount + liftG * 0.5f;
    b += liftAmount + liftB * 0.5f;
}

void ColorGradingPlugin::applyGamma(float& r, float& g, float& b) const {
    float gamma = static_cast<float>(getParameter("gamma"));
    float gammaR = static_cast<float>(getParameter("gammaR"));
    float gammaG = static_cast<float>(getParameter("gammaG"));
    float gammaB = static_cast<float>(getParameter("gammaB"));

    if (r > 0.0f) r = std::pow(r, 1.0f / (gamma * gammaR));
    if (g > 0.0f) g = std::pow(g, 1.0f / (gamma * gammaG));
    if (b > 0.0f) b = std::pow(b, 1.0f / (gamma * gammaB));
}

void ColorGradingPlugin::applyGain(float& r, float& g, float& b) const {
    float gain = static_cast<float>(getParameter("gain"));
    float gainR = static_cast<float>(getParameter("gainR"));
    float gainG = static_cast<float>(getParameter("gainG"));
    float gainB = static_cast<float>(getParameter("gainB"));

    r *= gain * gainR;
    g *= gain * gainG;
    b *= gain * gainB;
}

void ColorGradingPlugin::applySaturation(float& r, float& g, float& b) const {
    float sat = static_cast<float>(getParameter("saturation"));
    float lum = luminance(r, g, b);
    r = lum + (r - lum) * sat;
    g = lum + (g - lum) * sat;
    b = lum + (b - lum) * sat;
}

void ColorGradingPlugin::applyTemperature(float& r, float& g, float& b) const {
    float temp = static_cast<float>(getParameter("temperature"));
    r += temp * 0.15f;
    b -= temp * 0.15f;
}

void ColorGradingPlugin::applyContrast(float& r, float& g, float& b) const {
    float contrast = static_cast<float>(getParameter("contrast"));
    r = (r - 0.5f) * contrast + 0.5f;
    g = (g - 0.5f) * contrast + 0.5f;
    b = (b - 0.5f) * contrast + 0.5f;
}

void ColorGradingPlugin::applyHighlights(float& r, float& g, float& b) const {
    float amount = static_cast<float>(getParameter("highlights"));
    if (amount <= 0.0f) return;

    float lum = luminance(r, g, b);
    float highlightMask = std::clamp((lum - 0.5f) * 2.0f, 0.0f, 1.0f);
    float reduction = highlightMask * amount * 0.5f;

    r -= reduction;
    g -= reduction;
    b -= reduction;
}

void ColorGradingPlugin::applyShadows(float& r, float& g, float& b) const {
    float amount = static_cast<float>(getParameter("shadows"));
    if (amount <= 0.0f) return;

    float lum = luminance(r, g, b);
    float shadowMask = std::clamp((0.5f - lum) * 2.0f, 0.0f, 1.0f);
    float boost = shadowMask * amount * 0.5f;

    r += boost;
    g += boost;
    b += boost;
}

} // namespace FreeEffect
