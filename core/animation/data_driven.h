#pragma once
#include "../timeline/types.h"
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace FreeEffect {

using DataValue = std::variant<double, std::string, std::vector<double>>;

class DataDrivenAnimation {
public:
    bool loadFromJSON(const std::string& jsonStr);
    bool loadFromFile(const std::string& filePath);

    void bindToProperty(const std::string& dataPath, const std::string& propertyName);

    DataValue getValue(const std::string& path, double time) const;
    double getNumber(const std::string& path, double time) const;
    std::string getString(const std::string& path, double time) const;

    bool hasData() const { return !m_root.empty(); }
    std::vector<std::string> getDataPaths() const;

    std::map<std::string, std::string> getBindings() const { return m_bindings; }

private:
    std::map<std::string, DataValue> m_root;
    std::map<std::string, std::string> m_bindings;

    void parseAndFlatten(const std::string& jsonStr);
    void flattenObject(const std::map<std::string, DataValue>& obj, const std::string& prefix);
};

} // namespace FreeEffect
