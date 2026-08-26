#pragma once
#include "effect.h"
#include <memory>
#include <string>
#include <vector>

namespace FreeEffect {

class EffectSerializer {
public:
    struct EffectData {
        std::string effectName;
        UUID id;
        int order = 0;
        bool enabled = true;
        std::vector<std::pair<std::string, std::string>> stringParams;
        std::vector<std::pair<std::string, double>> floatParams;
        std::vector<std::pair<std::string, int>> intParams;
        std::vector<std::pair<std::string, bool>> boolParams;
        std::vector<std::pair<std::string, Color>> colorParams;
        std::vector<std::pair<std::string, Vec2>> vec2Params;
        std::vector<std::pair<std::string, int>> dropdownParams;
    };

    static std::string serializeEffect(const Effect& effect);
    static std::unique_ptr<Effect> deserializeEffect(const std::string& json);

    static std::string serializeEffectStack(const std::vector<EffectPtr>& effects);
    static std::vector<EffectPtr> deserializeEffectStack(const std::string& json);

    static EffectData extractData(const Effect& effect);
    static std::unique_ptr<Effect> constructFromData(const EffectData& data);
};

} // namespace FreeEffect
