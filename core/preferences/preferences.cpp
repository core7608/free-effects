#include "preferences.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace FreeEffect {

Preferences& Preferences::instance() {
    static Preferences prefs;
    return prefs;
}

std::string Preferences::getConfigPath() const {
    const char* home = std::getenv("HOME");
    if (!home) home = std::getenv("USERPROFILE");
    if (!home) return "freeeffect_prefs.cfg";
    return std::string(home) + "/.freeeffect/freeeffect_prefs.cfg";
}

void Preferences::set(const std::string& key, PrefValue value) {
    m_values[key] = value;
}

PrefValue Preferences::get(const std::string& key, PrefValue defaultValue) const {
    auto it = m_values.find(key);
    if (it != m_values.end()) return it->second;
    return defaultValue;
}

bool Preferences::getBool(const std::string& key, bool def) const {
    auto val = get(key, def);
    if (std::holds_alternative<bool>(val)) return std::get<bool>(val);
    return def;
}

int Preferences::getInt(const std::string& key, int def) const {
    auto val = get(key, def);
    if (std::holds_alternative<int>(val)) return std::get<int>(val);
    return def;
}

double Preferences::getDouble(const std::string& key, double def) const {
    auto val = get(key, def);
    if (std::holds_alternative<double>(val)) return std::get<double>(val);
    return def;
}

std::string Preferences::getString(const std::string& key, const std::string& def) const {
    auto val = get(key, def);
    if (std::holds_alternative<std::string>(val)) return std::get<std::string>(val);
    return def;
}

void Preferences::save() {
    std::string path = getConfigPath();
    std::ofstream file(path);
    if (!file.is_open()) return;

    for (const auto& [key, value] : m_values) {
        file << key << "=";
        if (std::holds_alternative<bool>(value)) {
            file << "bool:" << (std::get<bool>(value) ? "1" : "0");
        } else if (std::holds_alternative<int>(value)) {
            file << "int:" << std::get<int>(value);
        } else if (std::holds_alternative<double>(value)) {
            file << "double:" << std::get<double>(value);
        } else if (std::holds_alternative<std::string>(value)) {
            file << "string:" << std::get<std::string>(value);
        }
        file << "\n";
    }
}

void Preferences::load() {
    std::string path = getConfigPath();
    std::ifstream file(path);
    if (!file.is_open()) return;

    m_values.clear();
    std::string line;
    while (std::getline(file, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string valStr = line.substr(eq + 1);
        auto colon = valStr.find(':');
        if (colon == std::string::npos) continue;

        std::string type = valStr.substr(0, colon);
        std::string val = valStr.substr(colon + 1);

        if (type == "bool") m_values[key] = (val == "1");
        else if (type == "int") m_values[key] = std::stoi(val);
        else if (type == "double") m_values[key] = std::stod(val);
        else if (type == "string") m_values[key] = val;
    }
}

} // namespace FreeEffect
