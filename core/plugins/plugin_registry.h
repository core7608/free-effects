#pragma once

#include "plugin_interface.h"
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace FreeEffect {

struct PluginRegistryEntry {
    PluginInfo info;
    bool enabled = false;
    bool loaded = false;
    std::string category;
};

class PluginRegistry {
public:
    static PluginRegistry& getInstance();

    PluginRegistry(const PluginRegistry&) = delete;
    PluginRegistry& operator=(const PluginRegistry&) = delete;

    void registerPlugin(const PluginInfo& info);
    void unregisterPlugin(const UUID& pluginId);

    bool hasPlugin(const UUID& pluginId) const;
    bool hasPluginByName(const std::string& name) const;

    PluginRegistryEntry getEntry(const UUID& pluginId) const;
    PluginRegistryEntry getEntryByName(const std::string& name) const;

    std::vector<PluginRegistryEntry> getAllEntries() const;
    std::vector<PluginRegistryEntry> getEntriesByType(PluginType type) const;
    std::vector<PluginRegistryEntry> getEnabledEntries() const;
    std::vector<PluginRegistryEntry> getLoadedEntries() const;

    void setPluginEnabled(const UUID& pluginId, bool enabled);
    void setPluginLoaded(const UUID& pluginId, bool loaded);
    void setPluginCategory(const UUID& pluginId, const std::string& category);

    bool exportPluginList(const std::string& filePath) const;
    bool importPluginList(const std::string& filePath);

    void clear();

private:
    PluginRegistry() = default;
    ~PluginRegistry() = default;

    mutable std::mutex m_mutex;
    std::unordered_map<UUID, PluginRegistryEntry> m_entries;
    std::unordered_map<std::string, UUID> m_nameIndex;
};

} // namespace FreeEffect
