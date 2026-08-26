#include "template_manager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <stdexcept>

#ifdef HAS_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

namespace FreeEffect {

namespace fs = std::filesystem;

TemplateManager::TemplateManager() {
}

void TemplateManager::loadFromDirectory(const std::string& dirPath) {
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        return;
    }
    for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            loadFromFile(entry.path().string());
        }
    }
}

void TemplateManager::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    parseTemplateJson(ss.str());
}

const TemplateData* TemplateManager::getTemplate(const std::string& name) const {
    auto it = m_templates.find(name);
    return (it != m_templates.end()) ? &it->second : nullptr;
}

std::vector<std::string> TemplateManager::getCategories() const {
    std::vector<std::string> cats;
    for (const auto& [name, data] : m_templates) {
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

std::vector<std::string> TemplateManager::getTemplateNames(const std::string& category) const {
    std::vector<std::string> names;
    for (const auto& [name, data] : m_templates) {
        if (category.empty() || data.category == category) {
            names.push_back(name);
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::vector<const TemplateData*> TemplateManager::getTemplatesByCategory(const std::string& category) const {
    std::vector<const TemplateData*> result;
    for (const auto& [name, data] : m_templates) {
        if (category.empty() || data.category == category) {
            result.push_back(&data);
        }
    }
    return result;
}

std::unique_ptr<Composition> TemplateManager::createComposition(const std::string& templateName, const std::string& compName) {
    const TemplateData* tmpl = getTemplate(templateName);
    if (!tmpl) {
        return nullptr;
    }

    Resolution res;
    res.width = tmpl->width;
    res.height = tmpl->height;

    FrameRate fps;
    fps.fps = tmpl->fps;

    std::string name = compName.empty() ? tmpl->name : compName;
    auto comp = std::make_unique<Composition>(name, res, fps, tmpl->duration);
    comp->setBackgroundColor({tmpl->backgroundR, tmpl->backgroundG, tmpl->backgroundB, tmpl->backgroundA});

    for (const auto& tl : tmpl->layers) {
        LayerType lt = LayerType::Solid;
        if (tl.type == "video") lt = LayerType::Video;
        else if (tl.type == "image") lt = LayerType::Image;
        else if (tl.type == "text") lt = LayerType::Text;
        else if (tl.type == "shape") lt = LayerType::Shape;
        else if (tl.type == "null") lt = LayerType::Null;
        else if (tl.type == "solid") lt = LayerType::Solid;
        else if (tl.type == "adjustment") lt = LayerType::Adjustment;

        auto layer = comp->addLayer(tl.name, lt);
        layer->setStartTime(tl.startTime);
        layer->setDuration(tl.duration);

        layer->getPosition().setDefaultValue(tl.positionX);
        layer->getScale().setDefaultValue(tl.scaleX);
        layer->getRotation().setDefaultValue(tl.rotation);
        layer->getOpacity().setDefaultValue(tl.opacity);

        if (!tl.sourcePath.empty()) {
            layer->setSourcePath(tl.sourcePath);
        }

        if (tl.blendMode == "add") layer->setBlendMode(BlendMode::Add);
        else if (tl.blendMode == "multiply") layer->setBlendMode(BlendMode::Multiply);
        else if (tl.blendMode == "screen") layer->setBlendMode(BlendMode::Screen);
        else layer->setBlendMode(BlendMode::Normal);

        for (const auto& [propName, kfs] : tl.keyframes) {
            PropertyTrack* track = nullptr;
            if (propName == "Position") track = &layer->getPosition();
            else if (propName == "Scale") track = &layer->getScale();
            else if (propName == "Rotation") track = &layer->getRotation();
            else if (propName == "Opacity") track = &layer->getOpacity();

            if (track) {
                for (const auto& kf : kfs) {
                    auto timeIt = kf.find("time");
                    auto valIt = kf.find("value");
                    auto interpIt = kf.find("interpolation");
                    if (timeIt != kf.end() && valIt != kf.end()) {
                        InterpolationType interp = InterpolationType::Linear;
                        if (interpIt != kf.end()) {
                            int iv = static_cast<int>(interpIt->second);
                            if (iv >= 0 && iv <= 5) interp = static_cast<InterpolationType>(iv);
                        }
                        track->addKeyframe(Keyframe(timeIt->second, valIt->second, interp));
                    }
                }
            }
        }
    }

    return comp;
}

#ifdef HAS_NLOHMANN_JSON

void TemplateManager::parseTemplateJson(const std::string& jsonStr) {
    try {
        auto j = nlohmann::json::parse(jsonStr);
        if (j.is_array()) {
            for (const auto& item : j) {
                TemplateData td;
                td.name = item.value("name", "Untitled");
                td.description = item.value("description", "");
                td.category = item.value("category", "General");
                td.thumbnail = item.value("thumbnail", "");
                td.duration = item.value("duration", 10.0);
                td.width = item.value("width", 1920);
                td.height = item.value("height", 1080);
                td.fps = item.value("fps", 30.0);
                td.backgroundR = item.value("background_r", 0.0);
                td.backgroundG = item.value("background_g", 0.0);
                td.backgroundB = item.value("background_b", 0.0);
                td.backgroundA = item.value("background_a", 1.0);

                if (item.contains("layers") && item["layers"].is_array()) {
                    for (const auto& lj : item["layers"]) {
                        TemplateLayer tl;
                        tl.name = lj.value("name", "Layer");
                        tl.type = lj.value("type", "solid");
                        tl.startTime = lj.value("start_time", 0.0);
                        tl.duration = lj.value("duration", 5.0);
                        tl.positionX = lj.value("position_x", 960.0);
                        tl.positionY = lj.value("position_y", 540.0);
                        tl.scaleX = lj.value("scale_x", 100.0);
                        tl.scaleY = lj.value("scale_y", 100.0);
                        tl.rotation = lj.value("rotation", 0.0);
                        tl.opacity = lj.value("opacity", 100.0);
                        tl.text = lj.value("text", "");
                        tl.fontFamily = lj.value("font_family", "Arial");
                        tl.fontSize = lj.value("font_size", 48.0);
                        tl.textColorR = lj.value("text_color_r", 1.0);
                        tl.textColorG = lj.value("text_color_g", 1.0);
                        tl.textColorB = lj.value("text_color_b", 1.0);
                        tl.textColorA = lj.value("text_color_a", 1.0);
                        tl.sourcePath = lj.value("source_path", "");
                        tl.colorR = lj.value("color_r", 1.0);
                        tl.colorG = lj.value("color_g", 1.0);
                        tl.colorB = lj.value("color_b", 1.0);
                        tl.colorA = lj.value("color_a", 1.0);
                        tl.blendMode = lj.value("blend_mode", "normal");

                        if (lj.contains("effects") && lj["effects"].is_array()) {
                            for (const auto& ej : lj["effects"]) {
                                TemplateEffect te;
                                te.effectName = ej.value("effect_name", "");
                                if (ej.contains("parameters") && ej["parameters"].is_object()) {
                                    for (auto& [k, v] : ej["parameters"].items()) {
                                        te.parameters[k] = v.get<double>();
                                    }
                                }
                                tl.effects.push_back(te);
                            }
                        }

                        if (lj.contains("keyframes") && lj["keyframes"].is_object()) {
                            for (auto& [propName, kfArr] : lj["keyframes"].items()) {
                                std::vector<std::map<std::string, double>> kfs;
                                if (kfArr.is_array()) {
                                    for (const auto& kf : kfArr) {
                                        std::map<std::string, double> kfMap;
                                        for (auto& [kk, kv] : kf.items()) {
                                            if (kv.is_number()) {
                                                kfMap[kk] = kv.get<double>();
                                            }
                                        }
                                        kfs.push_back(kfMap);
                                    }
                                }
                                tl.keyframes[propName] = kfs;
                            }
                        }

                        td.layers.push_back(tl);
                    }
                }

                m_templates[td.name] = td;
            }
        }
    } catch (const std::exception&) {
    }
}

#else

void TemplateManager::parseTemplateJson(const std::string& jsonStr) {
    std::istringstream stream(jsonStr);
    std::string line;
    TemplateData current;
    bool inArray = false;
    TemplateLayer currentLayer;
    bool hasLayer = false;

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
            while (end < line.size() && (line[end] != ',' && line[end] != '}' && line[end] != ']' && line[end] != '\n')) end++;
            std::string valStr = line.substr(start, end - start);
            try {
                return std::stod(valStr);
            } catch (...) {
                return def;
            }
        };

        auto getIntValue = [&getDoubleValue](const std::string& key, const std::string& line, int def = 0) -> int {
            return static_cast<int>(getDoubleValue(key, line, static_cast<double>(def)));
        };

        if (trimmed.find("\"name\"") != std::string::npos && trimmed.find("\"layers\"") == std::string::npos) {
            std::string val = getStringValue("name", trimmed);
            if (!val.empty() && !inArray) {
                if (!current.name.empty()) {
                    m_templates[current.name] = current;
                }
                current = TemplateData();
                current.name = val;
            }
        }

        if (getStringValue("description", trimmed) != "" || trimmed.find("\"description\"") != std::string::npos) {
            std::string val = getStringValue("description", trimmed);
            if (!val.empty()) current.description = val;
        }
        if (trimmed.find("\"category\"") != std::string::npos) {
            std::string val = getStringValue("category", trimmed);
            if (!val.empty()) current.category = val;
        }
        if (trimmed.find("\"duration\"") != std::string::npos) {
            current.duration = getDoubleValue("duration", trimmed, 10.0);
        }
        if (trimmed.find("\"width\"") != std::string::npos) {
            current.width = getIntValue("width", trimmed, 1920);
        }
        if (trimmed.find("\"height\"") != std::string::npos) {
            current.height = getIntValue("height", trimmed, 1080);
        }
        if (trimmed.find("\"fps\"") != std::string::npos) {
            current.fps = getDoubleValue("fps", trimmed, 30.0);
        }
        if (trimmed.find("\"background_r\"") != std::string::npos) {
            current.backgroundR = getDoubleValue("background_r", trimmed);
        }
        if (trimmed.find("\"background_g\"") != std::string::npos) {
            current.backgroundG = getDoubleValue("background_g", trimmed);
        }
        if (trimmed.find("\"background_b\"") != std::string::npos) {
            current.backgroundB = getDoubleValue("background_b", trimmed);
        }
        if (trimmed.find("\"background_a\"") != std::string::npos) {
            current.backgroundA = getDoubleValue("background_a", trimmed, 1.0);
        }

        if (trimmed == "[") inArray = true;
        if (trimmed == "]" || trimmed == "],") {
            inArray = false;
        }

        if (trimmed.find("\"layers\"") != std::string::npos && trimmed.find('[') != std::string::npos) {
            continue;
        }

        if (trimmed.find("\"name\"") != std::string::npos && trimmed.find("\"type\"") != std::string::npos) {
            continue;
        }

        if (trimmed.find("\"type\"") != std::string::npos && trimmed.find("\"start_time\"") == std::string::npos) {
            continue;
        }

        if (trimmed.find("\"start_time\"") != std::string::npos) {
            if (hasLayer) {
                current.layers.push_back(currentLayer);
            }
            currentLayer = TemplateLayer();
            hasLayer = true;
            currentLayer.startTime = getDoubleValue("start_time", trimmed, 0.0);
        }
        if (hasLayer) {
            if (trimmed.find("\"duration\"") != std::string::npos && trimmed.find("start_time") == std::string::npos) {
                currentLayer.duration = getDoubleValue("duration", trimmed, 5.0);
            }
            if (trimmed.find("\"name\"") != std::string::npos && trimmed.find("\"type\"") == std::string::npos) {
                std::string val = getStringValue("name", trimmed);
                if (!val.empty()) currentLayer.name = val;
            }
            if (trimmed.find("\"type\"") != std::string::npos) {
                std::string val = getStringValue("type", trimmed);
                if (!val.empty()) currentLayer.type = val;
            }
            if (trimmed.find("\"position_x\"") != std::string::npos) {
                currentLayer.positionX = getDoubleValue("position_x", trimmed, 960.0);
            }
            if (trimmed.find("\"position_y\"") != std::string::npos) {
                currentLayer.positionY = getDoubleValue("position_y", trimmed, 540.0);
            }
            if (trimmed.find("\"scale_x\"") != std::string::npos) {
                currentLayer.scaleX = getDoubleValue("scale_x", trimmed, 100.0);
            }
            if (trimmed.find("\"scale_y\"") != std::string::npos) {
                currentLayer.scaleY = getDoubleValue("scale_y", trimmed, 100.0);
            }
            if (trimmed.find("\"rotation\"") != std::string::npos) {
                currentLayer.rotation = getDoubleValue("rotation", trimmed);
            }
            if (trimmed.find("\"opacity\"") != std::string::npos) {
                currentLayer.opacity = getDoubleValue("opacity", trimmed, 100.0);
            }
            if (trimmed.find("\"text\"") != std::string::npos && trimmed.find("\"text_") == std::string::npos) {
                currentLayer.text = getStringValue("text", trimmed);
            }
            if (trimmed.find("\"font_family\"") != std::string::npos) {
                currentLayer.fontFamily = getStringValue("font_family", trimmed);
            }
            if (trimmed.find("\"font_size\"") != std::string::npos) {
                currentLayer.fontSize = getDoubleValue("font_size", trimmed, 48.0);
            }
            if (trimmed.find("\"text_color_r\"") != std::string::npos) {
                currentLayer.textColorR = getDoubleValue("text_color_r", trimmed);
            }
            if (trimmed.find("\"text_color_g\"") != std::string::npos) {
                currentLayer.textColorG = getDoubleValue("text_color_g", trimmed);
            }
            if (trimmed.find("\"text_color_b\"") != std::string::npos) {
                currentLayer.textColorB = getDoubleValue("text_color_b", trimmed);
            }
            if (trimmed.find("\"text_color_a\"") != std::string::npos) {
                currentLayer.textColorA = getDoubleValue("text_color_a", trimmed, 1.0);
            }
            if (trimmed.find("\"source_path\"") != std::string::npos) {
                currentLayer.sourcePath = getStringValue("source_path", trimmed);
            }
            if (trimmed.find("\"color_r\"") != std::string::npos && trimmed.find("text_color") == std::string::npos) {
                currentLayer.colorR = getDoubleValue("color_r", trimmed);
            }
            if (trimmed.find("\"color_g\"") != std::string::npos && trimmed.find("text_color") == std::string::npos) {
                currentLayer.colorG = getDoubleValue("color_g", trimmed);
            }
            if (trimmed.find("\"color_b\"") != std::string::npos && trimmed.find("text_color") == std::string::npos) {
                currentLayer.colorB = getDoubleValue("color_b", trimmed);
            }
            if (trimmed.find("\"color_a\"") != std::string::npos && trimmed.find("text_color") == std::string::npos) {
                currentLayer.colorA = getDoubleValue("color_a", trimmed, 1.0);
            }
            if (trimmed.find("\"blend_mode\"") != std::string::npos) {
                currentLayer.blendMode = getStringValue("blend_mode", trimmed);
            }
        }

        if (trimmed == "}" || trimmed == "},") {
            if (hasLayer) {
                current.layers.push_back(currentLayer);
                currentLayer = TemplateLayer();
                hasLayer = false;
            }
        }
    }

    if (!current.name.empty()) {
        if (hasLayer) {
            current.layers.push_back(currentLayer);
        }
        m_templates[current.name] = current;
    }
}

#endif

} // namespace FreeEffect
