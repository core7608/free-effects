#pragma once

#include "preset_data.h"
#include "../../timeline/layer.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace FreeEffect {

class PresetManager {
public:
    PresetManager();
    ~PresetManager() = default;

    void loadFromDirectory(const std::string& dirPath);
    void loadFromFile(const std::string& filePath);

    const PresetData* getPreset(const std::string& name) const;
    std::vector<std::string> getCategories() const;
    std::vector<std::string> getPresetNames(const std::string& category = "") const;
    std::vector<const PresetData*> getPresetsByCategory(const std::string& category) const;

    void applyPreset(Layer& layer, const std::string& presetName) const;

private:
    void parsePresetJson(const std::string& jsonStr);

    std::unordered_map<std::string, PresetData> m_presets;
};

} // namespace FreeEffect
