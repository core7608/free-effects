#include "data_driven.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cctype>

namespace FreeEffect {

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n\"");
    size_t end = s.find_last_not_of(" \t\r\n\"");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

static bool isNumberStr(const std::string& s) {
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '-' || s[0] == '+') i = 1;
    bool hasDot = false;
    for (; i < s.size(); ++i) {
        if (s[i] == '.') { if (hasDot) return false; hasDot = true; }
        else if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    }
    return true;
}

struct JsonNode {
    enum Type { Number, String, Array, Object, Bool, Null } type;
    double numVal = 0;
    std::string strVal;
    std::vector<JsonNode> arrVal;
    std::map<std::string, JsonNode> objVal;
};

static void skipWS(const std::string& j, size_t& p) {
    while (p < j.size() && std::isspace(static_cast<unsigned char>(j[p]))) ++p;
}

static std::string parseStr(const std::string& j, size_t& p) {
    skipWS(j, p);
    if (p >= j.size() || j[p] != '"') return "";
    ++p;
    std::string r;
    while (p < j.size() && j[p] != '"') {
        if (j[p] == '\\' && p + 1 < j.size()) { ++p; r += j[p]; }
        else r += j[p];
        ++p;
    }
    if (p < j.size()) ++p;
    return r;
}

static std::string parseNumStr(const std::string& j, size_t& p) {
    skipWS(j, p);
    size_t start = p;
    if (p < j.size() && (j[p] == '-' || j[p] == '+')) ++p;
    while (p < j.size() && (std::isdigit(static_cast<unsigned char>(j[p])) || j[p] == '.')) ++p;
    if (p < j.size() && (j[p] == 'e' || j[p] == 'E')) {
        ++p;
        if (p < j.size() && (j[p] == '+' || j[p] == '-')) ++p;
        while (p < j.size() && std::isdigit(static_cast<unsigned char>(j[p]))) ++p;
    }
    return j.substr(start, p - start);
}

static JsonNode parseJson(const std::string& j, size_t& p);

static JsonNode parseJsonArray(const std::string& j, size_t& p) {
    JsonNode node;
    node.type = JsonNode::Array;
    skipWS(j, p);
    if (p >= j.size() || j[p] != '[') return node;
    ++p;
    skipWS(j, p);
    if (p < j.size() && j[p] == ']') { ++p; return node; }
    while (p < j.size()) {
        node.arrVal.push_back(parseJson(j, p));
        skipWS(j, p);
        if (p < j.size() && j[p] == ',') ++p;
        skipWS(j, p);
        if (p < j.size() && j[p] == ']') { ++p; break; }
    }
    return node;
}

static JsonNode parseJsonObject(const std::string& j, size_t& p) {
    JsonNode node;
    node.type = JsonNode::Object;
    skipWS(j, p);
    if (p >= j.size() || j[p] != '{') return node;
    ++p;
    skipWS(j, p);
    if (p < j.size() && j[p] == '}') { ++p; return node; }
    while (p < j.size()) {
        skipWS(j, p);
        std::string key = parseStr(j, p);
        skipWS(j, p);
        if (p < j.size() && j[p] == ':') ++p;
        node.objVal[key] = parseJson(j, p);
        skipWS(j, p);
        if (p < j.size() && j[p] == ',') ++p;
        skipWS(j, p);
        if (p < j.size() && j[p] == '}') { ++p; break; }
    }
    return node;
}

static JsonNode parseJson(const std::string& j, size_t& p) {
    skipWS(j, p);
    if (p >= j.size()) return JsonNode{JsonNode::Null};

    if (j[p] == '"') return JsonNode{JsonNode::String, 0, parseStr(j, p)};
    if (j[p] == '[') { ++p; return parseJsonArray(j, p); }
    if (j[p] == '{') { ++p; return parseJsonObject(j, p); }
    if (j[p] == 't') { p += 4; return JsonNode{JsonNode::Bool, 1.0}; }
    if (j[p] == 'f') { p += 5; return JsonNode{JsonNode::Bool, 0.0}; }
    if (j[p] == 'n') { p += 4; return JsonNode{JsonNode::Null}; }

    std::string numStr = parseNumStr(j, p);
    if (!numStr.empty() && isNumberStr(numStr)) {
        try { return JsonNode{JsonNode::Number, std::stod(numStr)}; }
        catch (...) { return JsonNode{JsonNode::Number, 0.0}; }
    }

    return JsonNode{JsonNode::Null};
}

static void flattenJsonNode(const JsonNode& node, const std::string& prefix,
                             std::map<std::string, DataValue>& out) {
    switch (node.type) {
        case JsonNode::Number:
            out[prefix] = node.numVal;
            break;
        case JsonNode::String:
            out[prefix] = node.strVal;
            break;
        case JsonNode::Bool:
            out[prefix] = node.numVal;
            break;
        case JsonNode::Array: {
            std::vector<double> arr;
            for (size_t i = 0; i < node.arrVal.size(); ++i) {
                const auto& elem = node.arrVal[i];
                if (elem.type == JsonNode::Number) {
                    arr.push_back(elem.numVal);
                } else {
                    std::string elemPath = prefix + "[" + std::to_string(i) + "]";
                    flattenJsonNode(elem, elemPath, out);
                    arr.push_back(0.0);
                }
            }
            if (!arr.empty()) {
                out[prefix] = arr;
            }
            break;
        }
        case JsonNode::Object:
            for (const auto& [key, val] : node.objVal) {
                std::string childPath = prefix.empty() ? key : prefix + "." + key;
                flattenJsonNode(val, childPath, out);
            }
            break;
        case JsonNode::Null:
            out[prefix] = 0.0;
            break;
    }
}

void DataDrivenAnimation::parseAndFlatten(const std::string& jsonStr) {
    m_root.clear();
    size_t pos = 0;
    JsonNode root = parseJson(jsonStr, pos);
    flattenJsonNode(root, "", m_root);
}

bool DataDrivenAnimation::loadFromJSON(const std::string& jsonStr) {
    parseAndFlatten(jsonStr);
    return !m_root.empty();
}

bool DataDrivenAnimation::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;
    std::ostringstream ss;
    ss << file.rdbuf();
    return loadFromJSON(ss.str());
}

void DataDrivenAnimation::bindToProperty(const std::string& dataPath, const std::string& propertyName) {
    m_bindings[dataPath] = propertyName;
}

DataValue DataDrivenAnimation::getValue(const std::string& path, double time) const {
    (void)time;
    auto it = m_root.find(path);
    if (it != m_root.end()) return it->second;

    for (const auto& [key, val] : m_root) {
        if (key.find(path) == 0 && key.size() > path.size() && key[path.size()] == '[') {
            return val;
        }
    }
    return 0.0;
}

double DataDrivenAnimation::getNumber(const std::string& path, double time) const {
    DataValue val = getValue(path, time);
    if (std::holds_alternative<double>(val)) return std::get<double>(val);
    if (std::holds_alternative<std::string>(val)) {
        try { return std::stod(std::get<std::string>(val)); } catch (...) { return 0.0; }
    }
    return 0.0;
}

std::string DataDrivenAnimation::getString(const std::string& path, double time) const {
    DataValue val = getValue(path, time);
    if (std::holds_alternative<std::string>(val)) return std::get<std::string>(val);
    if (std::holds_alternative<double>(val)) return std::to_string(std::get<double>(val));
    return "";
}

std::vector<std::string> DataDrivenAnimation::getDataPaths() const {
    std::vector<std::string> paths;
    for (const auto& [key, _] : m_root) {
        paths.push_back(key);
    }
    return paths;
}

} // namespace FreeEffect
