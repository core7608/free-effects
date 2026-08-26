#pragma once

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace FreeEffect {

using ParamValue = std::variant<double, std::string>;

struct PresetEffect {
    std::string effectName;
    std::map<std::string, ParamValue> parameters;
};

struct PresetData {
    std::string name;
    std::string description;
    std::string category;
    std::string thumbnail;
    std::vector<PresetEffect> effects;
};

} // namespace FreeEffect
