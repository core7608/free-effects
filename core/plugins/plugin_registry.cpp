#include "plugin_registry.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

namespace FreeEffect {

PluginRegistry& PluginRegistry::getInstance() {
    static PluginRegistry instance;
    return instance;
}

void PluginRegistry::registerPlugin(const PluginInfo& info) {
    std::lock_guard<std::mutex> lock(m_mutex);

    PluginRegistryEntry entry;
    entry.info = info;
    entry.enabled = false;
    entry.loaded = false;

    m_entries[info.id] = entry;
    m_nameIndex[info.name] = info.id;
}

void PluginRegistry::unregisterPlugin(const UUID& pluginId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_entries.find(pluginId);
    if (it != m_entries.end()) {
        m_nameIndex.erase(it->second.info.name);
        m_entries.erase(it);
    }
}

bool PluginRegistry::hasPlugin(const UUID& pluginId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_entries.find(pluginId) != m_entries.end();
}

bool PluginRegistry::hasPluginByName(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_nameIndex.find(name) != m_nameIndex.end();
}

PluginRegistryEntry PluginRegistry::getEntry(const UUID& pluginId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_entries.find(pluginId);
    if (it != m_entries.end()) {
        return it->second;
    }
    return {};
}

PluginRegistryEntry PluginRegistry::getEntryByName(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_nameIndex.find(name);
    if (it != m_nameIndex.end()) {
        auto entryIt = m_entries.find(it->second);
        if (entryIt != m_entries.end()) {
            return entryIt->second;
        }
    }
    return {};
}

std::vector<PluginRegistryEntry> PluginRegistry::getAllEntries() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PluginRegistryEntry> result;
    result.reserve(m_entries.size());
    for (const auto& [id, entry] : m_entries) {
        result.push_back(entry);
    }
    return result;
}

std::vector<PluginRegistryEntry> PluginRegistry::getEntriesByType(PluginType type) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PluginRegistryEntry> result;
    for (const auto& [id, entry] : m_entries) {
        if (entry.info.type == type) {
            result.push_back(entry);
        }
    }
    return result;
}

std::vector<PluginRegistryEntry> PluginRegistry::getEnabledEntries() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PluginRegistryEntry> result;
    for (const auto& [id, entry] : m_entries) {
        if (entry.enabled) {
            result.push_back(entry);
        }
    }
    return result;
}

std::vector<PluginRegistryEntry> PluginRegistry::getLoadedEntries() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PluginRegistryEntry> result;
    for (const auto& [id, entry] : m_entries) {
        if (entry.loaded) {
            result.push_back(entry);
        }
    }
    return result;
}

void PluginRegistry::setPluginEnabled(const UUID& pluginId, bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_entries.find(pluginId);
    if (it != m_entries.end()) {
        it->second.enabled = enabled;
    }
}

void PluginRegistry::setPluginLoaded(const UUID& pluginId, bool loaded) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_entries.find(pluginId);
    if (it != m_entries.end()) {
        it->second.loaded = loaded;
    }
}

void PluginRegistry::setPluginCategory(const UUID& pluginId, const std::string& category) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_entries.find(pluginId);
    if (it != m_entries.end()) {
        it->second.category = category;
    }
}

bool PluginRegistry::exportPluginList(const std::string& filePath) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    file << "# FreeEffect Plugin List\n";
    file << "# Format: name|version|type|enabled|category|filePath\n";

    for (const auto& [id, entry] : m_entries) {
        file << entry.info.name << "|"
             << entry.info.version << "|"
             << static_cast<int>(entry.info.type) << "|"
             << (entry.enabled ? "1" : "0") << "|"
             << entry.category << "|"
             << entry.info.filePath << "\n";
    }

    return file.good();
}

bool PluginRegistry::importPluginList(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string name, version, typeStr, enabledStr, category, filePath;
        if (std::getline(iss, name, '|') &&
            std::getline(iss, version, '|') &&
            std::getline(iss, typeStr, '|') &&
            std::getline(iss, enabledStr, '|') &&
            std::getline(iss, category, '|') &&
            std::getline(iss, filePath, '|')) {

            for (auto& [id, entry] : m_entries) {
                if (entry.info.name == name) {
                    entry.enabled = (enabledStr == "1");
                    entry.category = category;
                    break;
                }
            }
        }
    }

    return true;
}

void PluginRegistry::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries.clear();
    m_nameIndex.clear();
}

} // namespace FreeEffect
