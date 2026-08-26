#pragma once

#include "plugin_interface.h"
#include "plugin_registry.h"
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace FreeEffect {

class PluginManager {
public:
    static PluginManager& getInstance();

    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    PluginResult discoverPlugins();
    PluginResult loadPlugin(const std::string& filePath);
    PluginResult unloadPlugin(const UUID& pluginId);
    PluginResult loadAllPlugins();
    PluginResult unloadAllPlugins();

    PluginInterface* getPlugin(const UUID& pluginId);
    PluginInterface* getPluginByName(const std::string& name);

    std::vector<PluginInfo> getLoadedPluginInfos() const;
    std::vector<PluginInfo> getPluginsByType(PluginType type) const;

    bool isPluginLoaded(const UUID& pluginId) const;
    bool isPluginEnabled(const UUID& pluginId) const;
    PluginResult enablePlugin(const UUID& pluginId);
    PluginResult disablePlugin(const UUID& pluginId);

    void addSearchPath(const std::string& path);
    const std::vector<std::string>& getSearchPaths() const;
    void clearSearchPaths();

    using PluginCallback = std::function<void(const PluginInfo&)>;
    void setOnPluginLoaded(PluginCallback callback);
    void setOnPluginUnloaded(PluginCallback callback);

    PluginResult validatePluginAPI(const PluginInterface* plugin) const;

private:
    PluginManager();
    ~PluginManager();

    void loadDefaultSearchPaths();
    std::vector<std::string> scanDirectory(const std::string& dir) const;
    bool isSharedLibrary(const std::string& filename) const;
    UUID generatePluginId(const PluginInfo& info) const;

    void* loadSharedLibrary(const std::string& path);
    void unloadSharedLibrary(void* handle);
    PluginCreateFunc getCreateFunction(void* handle);
    PluginDestroyFunc getDestroyFunction(void* handle);

    mutable std::mutex m_mutex;
    std::vector<std::string> m_searchPaths;
    std::vector<std::unique_ptr<PluginEntry>> m_plugins;
    PluginRegistry& m_registry;

    PluginCallback m_onPluginLoaded;
    PluginCallback m_onPluginUnloaded;
};

} // namespace FreeEffect
