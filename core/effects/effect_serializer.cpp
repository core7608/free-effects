#include "effect_serializer.h"
#include "effect_registry.h"
#include <sstream>

namespace FreeEffect {

static void escapeString(std::string& s) {
    std::string result;
    result.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    s = result;
}

static std::string unescapeString(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            switch (s[i + 1]) {
                case '"': result += '"'; i++; break;
                case '\\': result += '\\'; i++; break;
                case 'n': result += '\n'; i++; break;
                case 'r': result += '\r'; i++; break;
                case 't': result += '\t'; i++; break;
                default: result += s[i]; break;
            }
        } else {
            result += s[i];
        }
    }
    return result;
}

static std::string colorToHex(const Color& c) {
    char buf[10];
    uint8_t r = static_cast<uint8_t>(std::clamp(c.r, 0.0, 255.0));
    uint8_t g = static_cast<uint8_t>(std::clamp(c.g, 0.0, 255.0));
    uint8_t b = static_cast<uint8_t>(std::clamp(c.b, 0.0, 255.0));
    uint8_t a = static_cast<uint8_t>(std::clamp(c.a * 255.0, 0.0, 255.0));
    snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x", r, g, b, a);
    return std::string(buf);
}

EffectSerializer::EffectData EffectSerializer::extractData(const Effect& effect) {
    EffectData data;
    data.effectName = effect.getName();
    data.id = effect.getId();
    data.order = effect.getOrder();
    data.enabled = effect.isEnabled();

    auto groups = effect.getParameterGroups();
    for (const auto& group : groups) {
        for (const auto& p : group.parameters) {
            switch (p.type) {
                case ParameterType::Float:
                    data.floatParams.push_back({p.name, p.floatValue});
                    break;
                case ParameterType::Int:
                    data.intParams.push_back({p.name, p.intValue});
                    break;
                case ParameterType::Bool:
                    data.boolParams.push_back({p.name, p.boolValue});
                    break;
                case ParameterType::Color:
                    data.colorParams.push_back({p.name, p.colorValue});
                    break;
                case ParameterType::Vec2:
                    data.vec2Params.push_back({p.name, p.vec2Value});
                    break;
                case ParameterType::Dropdown:
                    data.dropdownParams.push_back({p.name, p.dropdownValue});
                    break;
                case ParameterType::Angle:
                    data.floatParams.push_back({p.name, p.angleValue});
                    break;
                case ParameterType::String:
                    data.stringParams.push_back({p.name, p.stringValue});
                    break;
            }
        }
    }

    auto params = effect.getParameters();
    for (const auto& p : params) {
        switch (p.type) {
            case ParameterType::Float:
                data.floatParams.push_back({p.name, p.floatValue});
                break;
            case ParameterType::Int:
                data.intParams.push_back({p.name, p.intValue});
                break;
            case ParameterType::Bool:
                data.boolParams.push_back({p.name, p.boolValue});
                break;
            case ParameterType::Color:
                data.colorParams.push_back({p.name, p.colorValue});
                break;
            case ParameterType::Vec2:
                data.vec2Params.push_back({p.name, p.vec2Value});
                break;
            case ParameterType::Dropdown:
                data.dropdownParams.push_back({p.name, p.dropdownValue});
                break;
            case ParameterType::Angle:
                data.floatParams.push_back({p.name, p.angleValue});
                break;
            case ParameterType::String:
                data.stringParams.push_back({p.name, p.stringValue});
                break;
        }
    }
    return data;
}

std::unique_ptr<Effect> EffectSerializer::constructFromData(const EffectData& data) {
    auto effect = EffectRegistry::instance().create(data.effectName);
    if (!effect) return nullptr;

    effect->setId(data.id);
    effect->setOrder(data.order);
    effect->setEnabled(data.enabled);

    for (const auto& [name, val] : data.floatParams) {
        auto* p = effect->getParameter(name);
        if (p) {
            if (p->type == ParameterType::Angle) p->angleValue = val;
            else p->floatValue = val;
        }
    }
    for (const auto& [name, val] : data.intParams) {
        auto* p = effect->getParameter(name);
        if (p) p->intValue = val;
    }
    for (const auto& [name, val] : data.boolParams) {
        auto* p = effect->getParameter(name);
        if (p) p->boolValue = val;
    }
    for (const auto& [name, val] : data.colorParams) {
        auto* p = effect->getParameter(name);
        if (p) p->colorValue = val;
    }
    for (const auto& [name, val] : data.vec2Params) {
        auto* p = effect->getParameter(name);
        if (p) p->vec2Value = val;
    }
    for (const auto& [name, val] : data.dropdownParams) {
        auto* p = effect->getParameter(name);
        if (p) p->dropdownValue = val;
    }
    for (const auto& [name, val] : data.stringParams) {
        auto* p = effect->getParameter(name);
        if (p) p->stringValue = val;
    }
    return effect;
}

std::string EffectSerializer::serializeEffect(const Effect& effect) {
    auto data = extractData(effect);
    std::ostringstream ss;
    ss << "{";
    ss << "\"name\":\"" << data.effectName << "\",";
    ss << "\"id\":\"" << data.id << "\",";
    ss << "\"order\":" << data.order << ",";
    ss << "\"enabled\":" << (data.enabled ? "true" : "false");

    if (!data.floatParams.empty()) {
        ss << ",\"floats\":[";
        bool first = true;
        for (const auto& [name, val] : data.floatParams) {
            if (!first) ss << ",";
            ss << "{\"n\":\"" << name << "\",\"v\":" << val << "}";
            first = false;
        }
        ss << "]";
    }
    if (!data.intParams.empty()) {
        ss << ",\"ints\":[";
        bool first = true;
        for (const auto& [name, val] : data.intParams) {
            if (!first) ss << ",";
            ss << "{\"n\":\"" << name << "\",\"v\":" << val << "}";
            first = false;
        }
        ss << "]";
    }
    if (!data.boolParams.empty()) {
        ss << ",\"bools\":[";
        bool first = true;
        for (const auto& [name, val] : data.boolParams) {
            if (!first) ss << ",";
            ss << "{\"n\":\"" << name << "\",\"v\":" << (val ? "true" : "false") << "}";
            first = false;
        }
        ss << "]";
    }
    if (!data.colorParams.empty()) {
        ss << ",\"colors\":[";
        bool first = true;
        for (const auto& [name, val] : data.colorParams) {
            if (!first) ss << ",";
            ss << "{\"n\":\"" << name << "\",\"v\":\"" << colorToHex(val) << "\"}";
            first = false;
        }
        ss << "]";
    }
    if (!data.vec2Params.empty()) {
        ss << ",\"vecs\":[";
        bool first = true;
        for (const auto& [name, val] : data.vec2Params) {
            if (!first) ss << ",";
            ss << "{\"n\":\"" << name << "\",\"x\":" << val.x << ",\"y\":" << val.y << "}";
            first = false;
        }
        ss << "]";
    }
    if (!data.dropdownParams.empty()) {
        ss << ",\"dropdowns\":[";
        bool first = true;
        for (const auto& [name, val] : data.dropdownParams) {
            if (!first) ss << ",";
            ss << "{\"n\":\"" << name << "\",\"v\":" << val << "}";
            first = false;
        }
        ss << "]";
    }
    if (!data.stringParams.empty()) {
        ss << ",\"strings\":[";
        bool first = true;
        for (const auto& [name, val] : data.stringParams) {
            std::string escaped = val;
            escapeString(escaped);
            if (!first) ss << ",";
            ss << "{\"n\":\"" << name << "\",\"v\":\"" << escaped << "\"}";
            first = false;
        }
        ss << "]";
    }
    ss << "}";
    return ss.str();
}

std::string EffectSerializer::serializeEffectStack(const std::vector<EffectPtr>& effects) {
    std::ostringstream ss;
    ss << "[";
    bool first = true;
    for (const auto& effect : effects) {
        if (!effect) continue;
        if (!first) ss << ",";
        ss << serializeEffect(*effect);
        first = false;
    }
    ss << "]";
    return ss.str();
}

std::unique_ptr<Effect> EffectSerializer::deserializeEffect(const std::string& json) {
    EffectData data;

    auto findVal = [&json](const std::string& key) -> std::string {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        pos = json.find(':', pos);
        if (pos == std::string::npos) return "";
        pos++;
        while (pos < json.size() && json[pos] == ' ') pos++;
        if (pos >= json.size()) return "";
        if (json[pos] == '"') {
            pos++;
            auto end = json.find('"', pos);
            if (end == std::string::npos) return "";
            return json.substr(pos, end - pos);
        }
        auto end = json.find_first_of(",}", pos);
        if (end == std::string::npos) return json.substr(pos);
        return json.substr(pos, end - pos);
    };

    data.effectName = findVal("name");
    data.id = findVal("id");
    std::string orderStr = findVal("order");
    if (!orderStr.empty()) data.order = std::stoi(orderStr);
    std::string enabledStr = findVal("enabled");
    data.enabled = (enabledStr == "true");

    auto findFloatArray = [&json](const std::string& key, std::vector<std::pair<std::string, double>>& out) {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return;
        auto start = json.find('[', pos);
        if (start == std::string::npos) return;
        auto end = json.find(']', start);
        if (end == std::string::npos) return;
        std::string arr = json.substr(start + 1, end - start - 1);
        size_t p = 0;
        while (p < arr.size()) {
            auto nPos = arr.find("\"n\"", p);
            if (nPos == std::string::npos) break;
            auto nColon = arr.find(':', nPos);
            auto nQuote1 = arr.find('"', nColon + 1);
            auto nQuote2 = arr.find('"', nQuote1 + 1);
            std::string n = arr.substr(nQuote1 + 1, nQuote2 - nQuote1 - 1);
            auto vPos = arr.find("\"v\"", nQuote2);
            if (vPos == std::string::npos) break;
            auto vColon = arr.find(':', vPos);
            auto vEnd = arr.find_first_of(",}", vColon + 1);
            std::string vStr = arr.substr(vColon + 1, vEnd - vColon - 1);
            double v = std::stod(vStr);
            out.push_back({n, v});
            p = vEnd;
        }
    };

    findFloatArray("floats", data.floatParams);

    auto findIntArray = [&json](const std::string& key, std::vector<std::pair<std::string, int>>& out) {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return;
        auto start = json.find('[', pos);
        if (start == std::string::npos) return;
        auto end = json.find(']', start);
        if (end == std::string::npos) return;
        std::string arr = json.substr(start + 1, end - start - 1);
        size_t p = 0;
        while (p < arr.size()) {
            auto nPos = arr.find("\"n\"", p);
            if (nPos == std::string::npos) break;
            auto nColon = arr.find(':', nPos);
            auto nQuote1 = arr.find('"', nColon + 1);
            auto nQuote2 = arr.find('"', nQuote1 + 1);
            std::string n = arr.substr(nQuote1 + 1, nQuote2 - nQuote1 - 1);
            auto vPos = arr.find("\"v\"", nQuote2);
            if (vPos == std::string::npos) break;
            auto vColon = arr.find(':', vPos);
            auto vEnd = arr.find_first_of(",}", vColon + 1);
            std::string vStr = arr.substr(vColon + 1, vEnd - vColon - 1);
            int v = std::stoi(vStr);
            out.push_back({n, v});
            p = vEnd;
        }
    };

    findIntArray("ints", data.intParams);
    findIntArray("dropdowns", data.dropdownParams);

    auto findBoolArray = [&json](const std::string& key, std::vector<std::pair<std::string, bool>>& out) {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return;
        auto start = json.find('[', pos);
        if (start == std::string::npos) return;
        auto end = json.find(']', start);
        if (end == std::string::npos) return;
        std::string arr = json.substr(start + 1, end - start - 1);
        size_t p = 0;
        while (p < arr.size()) {
            auto nPos = arr.find("\"n\"", p);
            if (nPos == std::string::npos) break;
            auto nColon = arr.find(':', nPos);
            auto nQuote1 = arr.find('"', nColon + 1);
            auto nQuote2 = arr.find('"', nQuote1 + 1);
            std::string n = arr.substr(nQuote1 + 1, nQuote2 - nQuote1 - 1);
            auto vPos = arr.find("\"v\"", nQuote2);
            if (vPos == std::string::npos) break;
            auto vColon = arr.find(':', vPos);
            auto vEnd = arr.find_first_of(",}", vColon + 1);
            std::string vStr = arr.substr(vColon + 1, vEnd - vColon - 1);
            out.push_back({n, vStr == "true"});
            p = vEnd;
        }
    };

    findBoolArray("bools", data.boolParams);

    auto findColorArray = [&json](const std::string& key, std::vector<std::pair<std::string, Color>>& out) {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return;
        auto start = json.find('[', pos);
        if (start == std::string::npos) return;
        auto end = json.find(']', start);
        if (end == std::string::npos) return;
        std::string arr = json.substr(start + 1, end - start - 1);
        size_t p = 0;
        while (p < arr.size()) {
            auto nPos = arr.find("\"n\"", p);
            if (nPos == std::string::npos) break;
            auto nColon = arr.find(':', nPos);
            auto nQuote1 = arr.find('"', nColon + 1);
            auto nQuote2 = arr.find('"', nQuote1 + 1);
            std::string n = arr.substr(nQuote1 + 1, nQuote2 - nQuote1 - 1);
            auto vPos = arr.find("\"v\"", nQuote2);
            if (vPos == std::string::npos) break;
            auto vQuote1 = arr.find('"', vPos + 3);
            auto vQuote2 = arr.find('"', vQuote1 + 1);
            std::string hex = arr.substr(vQuote1 + 1, vQuote2 - vQuote1 - 1);
            Color c{0, 0, 0, 1};
            if (hex.size() >= 7) {
                unsigned int rv, gv, bv, av;
                if (hex.size() >= 9) {
                    sscanf(hex.c_str(), "#%02x%02x%02x%02x", &rv, &gv, &bv, &av);
                    c.a = av / 255.0;
                } else {
                    sscanf(hex.c_str(), "#%02x%02x%02x", &rv, &gv, &bv);
                }
                c.r = rv; c.g = gv; c.b = bv;
            }
            out.push_back({n, c});
            p = vQuote2 + 1;
        }
    };

    findColorArray("colors", data.colorParams);

    auto findVecArray = [&json](const std::string& key, std::vector<std::pair<std::string, Vec2>>& out) {
        auto pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos) return;
        auto start = json.find('[', pos);
        if (start == std::string::npos) return;
        auto end = json.find(']', start);
        if (end == std::string::npos) return;
        std::string arr = json.substr(start + 1, end - start - 1);
        size_t p = 0;
        while (p < arr.size()) {
            auto nPos = arr.find("\"n\"", p);
            if (nPos == std::string::npos) break;
            auto nColon = arr.find(':', nPos);
            auto nQuote1 = arr.find('"', nColon + 1);
            auto nQuote2 = arr.find('"', nQuote1 + 1);
            std::string n = arr.substr(nQuote1 + 1, nQuote2 - nQuote1 - 1);
            auto xPos = arr.find("\"x\"", nQuote2);
            if (xPos == std::string::npos) break;
            auto xColon = arr.find(':', xPos);
            auto xEnd = arr.find(',', xColon + 1);
            double x = std::stod(arr.substr(xColon + 1, xEnd - xColon - 1));
            auto yPos = arr.find("\"y\"", xEnd);
            if (yPos == std::string::npos) break;
            auto yColon = arr.find(':', yPos);
            auto yEnd = arr.find_first_of(",}", yColon + 1);
            double y = std::stod(arr.substr(yColon + 1, yEnd - yColon - 1));
            out.push_back({n, {x, y}});
            p = yEnd;
        }
    };

    findVecArray("vecs", data.vec2Params);

    return constructFromData(data);
}

std::vector<EffectPtr> EffectSerializer::deserializeEffectStack(const std::string& json) {
    std::vector<EffectPtr> result;
    size_t pos = 0;
    while (pos < json.size()) {
        auto start = json.find('{', pos);
        if (start == std::string::npos) break;
        int depth = 0;
        size_t end = start;
        for (; end < json.size(); end++) {
            if (json[end] == '{') depth++;
            else if (json[end] == '}') {
                depth--;
                if (depth == 0) break;
            }
        }
        if (depth != 0) break;
        std::string obj = json.substr(start, end - start + 1);
        auto effect = deserializeEffect(obj);
        if (effect) result.push_back(std::move(effect));
        pos = end + 1;
    }
    return result;
}

} // namespace FreeEffect
