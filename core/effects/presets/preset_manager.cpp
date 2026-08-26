#include "preset_manager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

#ifdef HAS_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

namespace FreeEffect {

namespace fs = std::filesystem;

PresetManager::PresetManager() {
}

void PresetManager::loadFromDirectory(const std::string& dirPath) {
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        return;
    }
    for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            loadFromFile(entry.path().string());
        }
    }
}

void PresetManager::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    parsePresetJson(ss.str());
}

const PresetData* PresetManager::getPreset(const std::string& name) const {
    auto it = m_presets.find(name);
    return (it != m_presets.end()) ? &it->second : nullptr;
}

std::vector<std::string> PresetManager::getCategories() const {
    std::vector<std::string> cats;
    for (const auto& [name, data] : m_presets) {
        bool found = false;
        for (const auto& c : cats) {
            if (c == data.category) { found = true; break; }
        }
        if (!found) {
            cats.push_back(data.category);
        }
    }
    std::sort(cats.begin(), cats.end());
    return cats;
}

std::vector<std::string> PresetManager::getPresetNames(const std::string& category) const {
    std::vector<std::string> names;
    for (const auto& [name, data] : m_presets) {
        if (category.empty() || data.category == category) {
            names.push_back(name);
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::vector<const PresetData*> PresetManager::getPresetsByCategory(const std::string& category) const {
    std::vector<const PresetData*> result;
    for (const auto& [name, data] : m_presets) {
        if (category.empty() || data.category == category) {
            result.push_back(&data);
        }
    }
    return result;
}

void PresetManager::applyPreset(Layer& layer, const std::string& presetName) const {
    const PresetData* preset = getPreset(presetName);
    if (!preset) {
        return;
    }

    for (const auto& pe : preset->effects) {
        for (const auto& [paramName, paramValue] : pe.parameters) {
            if (std::holds_alternative<double>(paramValue)) {
                double val = std::get<double>(paramValue);
                PropertyTrack* track = nullptr;

                if (paramName == "position") track = &layer.getPosition();
                else if (paramName == "scale") track = &layer.getScale();
                else if (paramName == "rotation") track = &layer.getRotation();
                else if (paramName == "opacity") track = &layer.getOpacity();

                if (track) {
                    track->setDefaultValue(val);
                }
            }
        }
    }
}

#ifdef HAS_NLOHMANN_JSON

void PresetManager::parsePresetJson(const std::string& jsonStr) {
    try {
        auto j = nlohmann::json::parse(jsonStr);
        if (j.is_array()) {
            for (const auto& item : j) {
                PresetData pd;
                pd.name = item.value("name", "Untitled");
                pd.description = item.value("description", "");
                pd.category = item.value("category", "General");
                pd.thumbnail = item.value("thumbnail", "");

                if (item.contains("effects") && item["effects"].is_array()) {
                    for (const auto& ej : item["effects"]) {
                        PresetEffect pe;
                        pe.effectName = ej.value("effect_name", "");
                        if (ej.contains("parameters") && ej["parameters"].is_object()) {
                            for (auto& [k, v] : ej["parameters"].items()) {
                                if (v.is_number()) {
                                    pe.parameters[k] = v.get<double>();
                                } else if (v.is_string()) {
                                    pe.parameters[k] = v.get<std::string>();
                                }
                            }
                        }
                        pd.effects.push_back(pe);
                    }
                }

                m_presets[pd.name] = pd;
            }
        }
    } catch (const std::exception&) {
    }
}

#else

void PresetManager::parsePresetJson(const std::string& jsonStr) {
    std::istringstream stream(jsonStr);
    std::string line;
    PresetData current;
    PresetEffect currentEffect;
    bool hasEffect = false;

    while (std::getline(stream, line)) {
        auto trim = [](std::string s) {
            size_t start = s.find_first_not_of(" \t\r\n");
            size_t end = s.find_last_not_of(" \t\r\n");
            return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
        };

        std::string trimmed = trim(line);

        auto getStringValue = [](const std::string& key, const std::string& line) -> std::string {
            size_t pos = line.find("\"" + key + "\"");
            if (pos == std::string::npos) return "";
            pos = line.find(':', pos);
            if (pos == std::string::npos) return "";
            size_t start = line.find('"', pos + 1);
            if (start == std::string::npos) return "";
            size_t end = line.find('"', start + 1);
            if (end == std::string::npos) return "";
            return line.substr(start + 1, end - start - 1);
        };

        auto getDoubleValue = [](const std::string& key, const std::string& line, double def = 0.0) -> double {
            size_t pos = line.find("\"" + key + "\"");
            if (pos == std::string::npos) return def;
            pos = line.find(':', pos);
            if (pos == std::string::npos) return def;
            size_t start = pos + 1;
            while (start < line.size() && (line[start] == ' ' || line[start] == '\t')) start++;
            size_t end = start;
            while (end < line.size() && (line[end] != ',' && line[end] != '}' && line[end] != ']')) end++;
            std::string valStr = line.substr(start, end - start);
            try {
                return std::stod(valStr);
            } catch (...) {
                return def;
            }
        };

        if (trimmed.find("\"name\"") != std::string::npos && trimmed.find("\"effects\"") == std::string::npos) {
            std::string val = getStringValue("name", trimmed);
            if (!val.empty() && trimmed.find("effect_name") == std::string::npos) {
                if (!current.name.empty()) {
                    if (hasEffect) {
                        current.effects.push_back(currentEffect);
                        currentEffect = PresetEffect();
                        hasEffect = false;
                    }
                    m_presets[current.name] = current;
                }
                current = PresetData();
                current.name = val;
            }
        }
        if (trimmed.find("\"description\"") != std::string::npos) {
            std::string val = getStringValue("description", trimmed);
            if (!val.empty()) current.description = val;
        }
        if (trimmed.find("\"category\"") != std::string::npos && trimmed.find("effect") == std::string::npos) {
            std::string val = getStringValue("category", trimmed);
            if (!val.empty()) current.category = val;
        }

        if (trimmed.find("\"effect_name\"") != std::string::npos) {
            if (hasEffect) {
                current.effects.push_back(currentEffect);
            }
            currentEffect = PresetEffect();
            hasEffect = true;
            std::string val = getStringValue("effect_name", trimmed);
            currentEffect.effectName = val;
        }

        if (hasEffect && trimmed.find("\"parameters\"") == std::string::npos &&
            trimmed.find('"') != std::string::npos) {
            size_t colonPos = trimmed.find(':');
            if (colonPos != std::string::npos) {
                size_t keyStart = trimmed.find('"');
                size_t keyEnd = trimmed.find('"', keyStart + 1);
                if (keyStart != std::string::npos && keyEnd != std::string::npos) {
                    std::string key = trimmed.substr(keyStart + 1, keyEnd - keyStart - 1);
                    if (key != "parameters" && key != "effect_name" && key != "name") {
                        size_t valStart = colonPos + 1;
                        while (valStart < trimmed.size() && (trimmed[valStart] == ' ' || trimmed[valStart] == '\t')) valStart++;
                        if (valStart < trimmed.size() && trimmed[valStart] == '"') {
                            size_t valEnd = trimmed.find('"', valStart + 1);
                            if (valEnd != std::string::npos) {
                                currentEffect.parameters[key] = trimmed.substr(valStart + 1, valEnd - valStart - 1);
                            }
                        } else {
                            size_t valEnd = valStart;
                            while (valEnd < trimmed.size() && trimmed[valEnd] != ',' && trimmed[valEnd] != '}' && trimmed[valEnd] != ']') valEnd++;
                            std::string valStr = trimmed.substr(valStart, valEnd - valStart);
                            try {
                                currentEffect.parameters[key] = std::stod(valStr);
                            } catch (...) {
                            }
                        }
                    }
                }
            }
        }

        if (trimmed == "}" || trimmed == "},") {
            if (hasEffect) {
                current.effects.push_back(currentEffect);
                currentEffect = PresetEffect();
                hasEffect = false;
            }
        }
    }

    if (!current.name.empty()) {
        if (hasEffect) {
            current.effects.push_back(currentEffect);
        }
        m_presets[current.name] = current;
    }
}

#endif

} // namespace FreeEffect
