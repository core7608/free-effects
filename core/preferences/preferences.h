#pragma once
#include <string>
#include <map>
#include <variant>

namespace FreeEffect {

using PrefValue = std::variant<bool, int, double, std::string>;

class Preferences {
public:
    static Preferences& instance();
    
    void set(const std::string& key, PrefValue value);
    PrefValue get(const std::string& key, PrefValue defaultValue = {}) const;
    bool getBool(const std::string& key, bool def = false) const;
    int getInt(const std::string& key, int def = 0) const;
    double getDouble(const std::string& key, double def = 0.0) const;
    std::string getString(const std::string& key, const std::string& def = "") const;
    
    void save();
    void load();
    
    // AE-style preference categories
    static constexpr const char* kGeneral = "general.";
    static constexpr const char* kDisplay = "display.";
    static constexpr const char* kImport = "import.";
    static constexpr const char* kOutput = "output.";
    static constexpr const char* kGrid = "grid.";
    static constexpr const char* kLabels = "labels.";
    static constexpr const char* kMemory = "memory.";
    static constexpr const char* kVideoPreview = "videoPreview.";

private:
    Preferences() { load(); }
    std::map<std::string, PrefValue> m_values;
    std::string getConfigPath() const;
};

} // namespace FreeEffect
