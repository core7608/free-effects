#pragma once

#include "../rendering/renderer.h"
#include "../timeline/types.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace FreeEffect {

enum class ParameterType {
    Float,
    Int,
    Bool,
    Color,
    Vec2,
    Dropdown,
    Angle,
    String
};

struct EffectParameter {
    std::string name;
    std::string label;
    ParameterType type = ParameterType::Float;
    bool keyframeable = true;
    bool animatable = false;

    double floatMin = 0.0;
    double floatMax = 1.0;
    double floatDefault = 0.0;
    double floatValue = 0.0;

    int intMin = 0;
    int intMax = 100;
    int intDefault = 0;
    int intValue = 0;

    bool boolDefault = false;
    bool boolValue = false;

    Color colorDefault{0.0, 0.0, 0.0, 1.0};
    Color colorValue{0.0, 0.0, 0.0, 1.0};

    Vec2 vec2Default{0.0, 0.0};
    Vec2 vec2Value{0.0, 0.0};

    std::vector<std::string> dropdownOptions;
    int dropdownDefault = 0;
    int dropdownValue = 0;

    double angleDefault = 0.0;
    double angleValue = 0.0;

    std::string stringDefault;
    std::string stringValue;

    std::vector<PropertyTrack> tracks;

    static EffectParameter makeFloat(const std::string& n, const std::string& l,
                                     double min, double max, double def, bool keyframeable = true) {
        EffectParameter p;
        p.name = n; p.label = l; p.type = ParameterType::Float;
        p.floatMin = min; p.floatMax = max; p.floatDefault = def; p.floatValue = def;
        p.keyframeable = keyframeable;
        if (keyframeable) p.tracks.emplace_back(n);
        return p;
    }

    static EffectParameter makeInt(const std::string& n, const std::string& l,
                                   int min, int max, int def, bool keyframeable = true) {
        EffectParameter p;
        p.name = n; p.label = l; p.type = ParameterType::Int;
        p.intMin = min; p.intMax = max; p.intDefault = def; p.intValue = def;
        p.keyframeable = keyframeable;
        if (keyframeable) p.tracks.emplace_back(n);
        return p;
    }

    static EffectParameter makeBool(const std::string& n, const std::string& l, bool def) {
        EffectParameter p;
        p.name = n; p.label = l; p.type = ParameterType::Bool;
        p.boolDefault = def; p.boolValue = def;
        p.keyframeable = false;
        return p;
    }

    static EffectParameter makeColor(const std::string& n, const std::string& l,
                                     Color def, bool keyframeable = false) {
        EffectParameter p;
        p.name = n; p.label = l; p.type = ParameterType::Color;
        p.colorDefault = def; p.colorValue = def;
        p.keyframeable = keyframeable;
        return p;
    }

    static EffectParameter makeVec2(const std::string& n, const std::string& l,
                                    Vec2 def, bool keyframeable = false) {
        EffectParameter p;
        p.name = n; p.label = l; p.type = ParameterType::Vec2;
        p.vec2Default = def; p.vec2Value = def;
        p.keyframeable = keyframeable;
        return p;
    }

    static EffectParameter makeDropdown(const std::string& n, const std::string& l,
                                        const std::vector<std::string>& opts, int def) {
        EffectParameter p;
        p.name = n; p.label = l; p.type = ParameterType::Dropdown;
        p.dropdownOptions = opts; p.dropdownDefault = def; p.dropdownValue = def;
        p.keyframeable = false;
        return p;
    }

    static EffectParameter makeAngle(const std::string& n, const std::string& l,
                                     double def, bool keyframeable = true) {
        EffectParameter p;
        p.name = n; p.label = l; p.type = ParameterType::Angle;
        p.angleDefault = def; p.angleValue = def;
        p.keyframeable = keyframeable;
        if (keyframeable) p.tracks.emplace_back(n);
        return p;
    }

    static EffectParameter makeString(const std::string& n, const std::string& l,
                                      const std::string& def) {
        EffectParameter p;
        p.name = n; p.label = l; p.type = ParameterType::String;
        p.stringDefault = def; p.stringValue = def;
        p.keyframeable = false;
        return p;
    }

    void resetToDefault() {
        floatValue = floatDefault;
        intValue = intDefault;
        boolValue = boolDefault;
        colorValue = colorDefault;
        vec2Value = vec2Default;
        dropdownValue = dropdownDefault;
        angleValue = angleDefault;
        stringValue = stringDefault;
    }
};

struct ParameterGroup {
    std::string name;
    std::vector<EffectParameter> parameters;
};

class Effect {
public:
    Effect();
    virtual ~Effect() = default;

    const UUID& getId() const { return m_id; }
    void setId(const UUID& id) { m_id = id; }

    virtual std::string getName() const = 0;
    virtual std::string getCategory() const = 0;
    virtual std::string getSubCategory() const { return ""; }

    int getOrder() const { return m_order; }
    void setOrder(int order) { m_order = order; }

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    virtual std::vector<ParameterGroup> getParameterGroups() const { return {}; }
    virtual std::vector<EffectParameter> getParameters() const { return {}; }

    EffectParameter* getParameter(const std::string& name);
    const EffectParameter* getParameter(const std::string& name) const;

    void setParameterValue(const std::string& name, double value);
    void setParameterValue(const std::string& name, int value);
    void setParameterValue(const std::string& name, bool value);
    void setParameterValue(const std::string& name, const Color& value);
    void setParameterValue(const std::string& name, const Vec2& value);

    double getFloatParam(const std::string& name) const;
    int getIntParam(const std::string& name) const;
    bool getBoolParam(const std::string& name) const;
    Color getColorParam(const std::string& name) const;
    Vec2 getVec2Param(const std::string& name) const;
    int getDropdownParam(const std::string& name) const;
    double getAngleParam(const std::string& name) const;

    void addParameter(const EffectParameter& param);
    void addParameterGroup(const ParameterGroup& group);

    virtual void render(PixelBuffer& buffer, double time) = 0;
    virtual void apply(PixelBuffer& buffer, double time) { render(buffer, time); }

    virtual std::unique_ptr<Effect> clone() const = 0;

    virtual void updateParameterTracks(double time);

protected:
    void applyToBuffer(PixelBuffer& target, const PixelBuffer& source);
    void copyBuffer(PixelBuffer& dst, const PixelBuffer& src);

    UUID m_id;
    int m_order = 0;
    bool m_enabled = true;
    std::vector<ParameterGroup> m_parameterGroups;
    std::vector<EffectParameter> m_parameters;
};

using EffectPtr = std::unique_ptr<Effect>;

} // namespace FreeEffect
