#include "effect.h"
#include <cmath>
#include <algorithm>

namespace FreeEffect {

Effect::Effect() : m_id(generateUUID()) {}

EffectParameter* Effect::getParameter(const std::string& name) {
    for (auto& group : m_parameterGroups) {
        for (auto& p : group.parameters) {
            if (p.name == name) return &p;
        }
    }
    for (auto& p : m_parameters) {
        if (p.name == name) return &p;
    }
    return nullptr;
}

const EffectParameter* Effect::getParameter(const std::string& name) const {
    for (const auto& group : m_parameterGroups) {
        for (const auto& p : group.parameters) {
            if (p.name == name) return &p;
        }
    }
    for (const auto& p : m_parameters) {
        if (p.name == name) return &p;
    }
    return nullptr;
}

void Effect::setParameterValue(const std::string& name, double value) {
    auto* p = getParameter(name);
    if (p && p->type == ParameterType::Float) {
        p->floatValue = std::clamp(value, p->floatMin, p->floatMax);
    }
}

void Effect::setParameterValue(const std::string& name, int value) {
    auto* p = getParameter(name);
    if (p && p->type == ParameterType::Int) {
        p->intValue = std::clamp(value, p->intMin, p->intMax);
    }
}

void Effect::setParameterValue(const std::string& name, bool value) {
    auto* p = getParameter(name);
    if (p && p->type == ParameterType::Bool) {
        p->boolValue = value;
    }
}

void Effect::setParameterValue(const std::string& name, const Color& value) {
    auto* p = getParameter(name);
    if (p && p->type == ParameterType::Color) {
        p->colorValue = value;
    }
}

void Effect::setParameterValue(const std::string& name, const Vec2& value) {
    auto* p = getParameter(name);
    if (p && p->type == ParameterType::Vec2) {
        p->vec2Value = value;
    }
}

double Effect::getFloatParam(const std::string& name) const {
    auto* p = getParameter(name);
    return p ? p->floatValue : 0.0;
}

int Effect::getIntParam(const std::string& name) const {
    auto* p = getParameter(name);
    return p ? p->intValue : 0;
}

bool Effect::getBoolParam(const std::string& name) const {
    auto* p = getParameter(name);
    return p ? p->boolValue : false;
}

Color Effect::getColorParam(const std::string& name) const {
    auto* p = getParameter(name);
    return p ? p->colorValue : Color{0, 0, 0, 1};
}

Vec2 Effect::getVec2Param(const std::string& name) const {
    auto* p = getParameter(name);
    return p ? p->vec2Value : Vec2{0, 0};
}

int Effect::getDropdownParam(const std::string& name) const {
    auto* p = getParameter(name);
    return p ? p->dropdownValue : 0;
}

double Effect::getAngleParam(const std::string& name) const {
    auto* p = getParameter(name);
    return p ? p->angleValue : 0.0;
}

void Effect::addParameter(const EffectParameter& param) {
    m_parameters.push_back(param);
}

void Effect::addParameterGroup(const ParameterGroup& group) {
    m_parameterGroups.push_back(group);
}

void Effect::updateParameterTracks(double time) {
    for (auto& group : m_parameterGroups) {
        for (auto& p : group.parameters) {
            for (auto& track : p.tracks) {
                if (track.hasKeyframes()) {
                    double val = track.getValueAtTime(time);
                    switch (p.type) {
                        case ParameterType::Float:
                        case ParameterType::Angle:
                            p.floatValue = val;
                            break;
                        case ParameterType::Int:
                            p.intValue = static_cast<int>(val);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    for (auto& p : m_parameters) {
        for (auto& track : p.tracks) {
            if (track.hasKeyframes()) {
                double val = track.getValueAtTime(time);
                switch (p.type) {
                    case ParameterType::Float:
                    case ParameterType::Angle:
                        p.floatValue = val;
                        break;
                    case ParameterType::Int:
                        p.intValue = static_cast<int>(val);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void Effect::applyToBuffer(PixelBuffer& target, const PixelBuffer& source) {
    copyBuffer(target, source);
}

void Effect::copyBuffer(PixelBuffer& dst, const PixelBuffer& src) {
    if (dst.width != src.width || dst.height != src.height) {
        dst.resize(src.width, src.height);
    }
    dst.data = src.data;
}

} // namespace FreeEffect
