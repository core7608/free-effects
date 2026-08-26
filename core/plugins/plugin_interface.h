#pragma once

#include "../timeline/types.h"
#include <cstdint>
#include <string>
#include <vector>

namespace FreeEffect {

enum class PluginType {
    Effect,
    Generator,
    Transition,
    Extension
};

enum class PluginAPIVersion : uint32_t {
    v1_0 = 100,
    v1_1 = 101,
    v2_0 = 200
};

struct PluginResult {
    bool success = true;
    std::string errorMessage;

    static PluginResult ok() {
        PluginResult r;
        r.success = true;
        return r;
    }

    static PluginResult error(const std::string& msg) {
        PluginResult r;
        r.success = false;
        r.errorMessage = msg;
        return r;
    }
};

struct PluginParameter {
    std::string name;
    std::string displayName;
    std::string description;
    double minValue = 0.0;
    double maxValue = 1.0;
    double defaultValue = 0.0;
    double currentValue = 0.0;
    bool animatable = true;

    enum class Type {
        Float,
        Int,
        Bool,
        Color,
        String,
        Vec2,
        Enum
    };

    Type type = Type::Float;
    std::vector<std::string> enumOptions;
};

struct PluginInfo {
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    PluginType type = PluginType::Effect;
    PluginAPIVersion apiVersion = PluginAPIVersion::v1_0;
    std::string filePath;
    UUID id;
};

class PluginInterface {
public:
    virtual ~PluginInterface() = default;

    virtual PluginResult initialize() = 0;
    virtual PluginResult shutdown() = 0;

    virtual const char* getName() const = 0;
    virtual const char* getVersion() const = 0;
    virtual const char* getDescription() const = 0;
    virtual const char* getAuthor() const = 0;
    virtual PluginAPIVersion getAPIVersion() const = 0;
    virtual PluginType getType() const = 0;

    virtual uint32_t getParameterCount() const { return 0; }
    virtual PluginParameter getParameter(uint32_t index) const { return {}; }
    virtual PluginResult setParameter(const std::string& name, double value) { return PluginResult::ok(); }
    virtual double getParameter(const std::string& name) const { return 0.0; }

    virtual bool needsProcessing() const { return false; }
    virtual PluginResult process(float* buffer, int width, int height, int channels, double time) {
        return PluginResult::ok();
    }
};

using PluginCreateFunc = PluginInterface* (*)();
using PluginDestroyFunc = void (*)(PluginInterface*);

struct PluginEntry {
    PluginInterface* instance = nullptr;
    PluginCreateFunc createFunc = nullptr;
    PluginDestroyFunc destroyFunc = nullptr;
    void* handle = nullptr;
    PluginInfo info;
    bool enabled = true;
    bool loaded = false;
};

} // namespace FreeEffect
