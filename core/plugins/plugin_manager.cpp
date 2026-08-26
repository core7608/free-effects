#include "plugin_manager.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace FreeEffect {

namespace fs = std::filesystem;

PluginManager::PluginManager() : m_registry(PluginRegistry::getInstance()) {
    loadDefaultSearchPaths();
}

PluginManager::~PluginManager() {
    unloadAllPlugins();
}

PluginManager& PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

void PluginManager::loadDefaultSearchPaths() {
    const char* home = nullptr;
#ifdef _WIN32
    home = std::getenv("USERPROFILE");
#else
    home = std::getenv("HOME");
#endif
    if (home) {
        m_searchPaths.push_back(std::string(home) + "/.freeeffect/plugins");
    }
    m_searchPaths.push_back("./plugins");
    m_searchPaths.push_back("/usr/local/lib/freeeffect/plugins");
}

void PluginManager::addSearchPath(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (std::find(m_searchPaths.begin(), m_searchPaths.end(), path) == m_searchPaths.end()) {
        m_searchPaths.push_back(path);
    }
}

const std::vector<std::string>& PluginManager::getSearchPaths() const {
    return m_searchPaths;
}

void PluginManager::clearSearchPaths() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_searchPaths.clear();
}

void PluginManager::setOnPluginLoaded(PluginCallback callback) {
    m_onPluginLoaded = std::move(callback);
}

void PluginManager::setOnPluginUnloaded(PluginCallback callback) {
    m_onPluginUnloaded = std::move(callback);
}

bool PluginManager::isSharedLibrary(const std::string& filename) const {
#ifdef _WIN32
    return filename.size() > 4 &&
           (filename.substr(filename.size() - 4) == ".dll");
#elif defined(__APPLE__)
    return filename.size() > 6 &&
           (filename.substr(filename.size() - 6) == ".dylib");
#else
    return filename.size() > 3 &&
           (filename.substr(filename.size() - 3) == ".so");
#endif
}

std::vector<std::string> PluginManager::scanDirectory(const std::string& dir) const {
    std::vector<std::string> result;
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) return result;

    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (entry.is_regular_file() && isSharedLibrary(entry.path().filename().string())) {
            result.push_back(entry.path().string());
        }
    }
    return result;
}

PluginResult PluginManager::discoverPlugins() {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<std::string> allLibs;
    for (const auto& path : m_searchPaths) {
        auto libs = scanDirectory(path);
        allLibs.insert(allLibs.end(), libs.begin(), libs.end());
    }

    int discovered = 0;
    for (const auto& libPath : allLibs) {
        bool alreadyRegistered = false;
        for (const auto& plugin : m_plugins) {
            if (plugin->info.filePath == libPath) {
                alreadyRegistered = true;
                break;
            }
        }
        if (!alreadyRegistered) {
            PluginEntry entry;
            entry.info.filePath = libPath;
            entry.info.id = generatePluginId(entry.info);
            m_plugins.push_back(std::make_unique<PluginEntry>(std::move(entry)));
            m_registry.registerPlugin(m_plugins.back()->info);
            discovered++;
        }
    }

    std::cout << "Discovered " << discovered << " new plugin(s)" << std::endl;
    return PluginResult::ok();
}

PluginResult PluginManager::loadPlugin(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& entry : m_plugins) {
        if (entry->info.filePath == filePath && entry->loaded) {
            return PluginResult::error("Plugin already loaded: " + filePath);
        }
    }

    void* handle = loadSharedLibrary(filePath);
    if (!handle) {
        return PluginResult::error("Failed to load shared library: " + filePath);
    }

    auto createFunc = getCreateFunction(handle);
    auto destroyFunc = getDestroyFunction(handle);
    if (!createFunc || !destroyFunc) {
        unloadSharedLibrary(handle);
        return PluginResult::error("Invalid plugin exports: " + filePath);
    }

    PluginInterface* instance = createFunc();
    if (!instance) {
        unloadSharedLibrary(handle);
        return PluginResult::error("Plugin create function returned null: " + filePath);
    }

    PluginInfo info;
    info.name = instance->getName();
    info.version = instance->getVersion();
    info.description = instance->getDescription();
    info.author = instance->getAuthor();
    info.type = instance->getType();
    info.apiVersion = instance->getAPIVersion();
    info.filePath = filePath;
    info.id = generatePluginId(info);

    PluginResult apiCheck = validatePluginAPI(instance);
    if (!apiCheck.success) {
        destroyFunc(instance);
        unloadSharedLibrary(handle);
        return apiCheck;
    }

    PluginResult initResult = instance->initialize();
    if (!initResult.success) {
        destroyFunc(instance);
        unloadSharedLibrary(handle);
        return initResult;
    }

    auto* pluginEntry = new PluginEntry();
    pluginEntry->instance = instance;
    pluginEntry->createFunc = createFunc;
    pluginEntry->destroyFunc = destroyFunc;
    pluginEntry->handle = handle;
    pluginEntry->info = info;
    pluginEntry->loaded = true;
    pluginEntry->enabled = true;
    m_plugins.push_back(std::unique_ptr<PluginEntry>(pluginEntry));

    m_registry.registerPlugin(info);
    m_registry.setPluginEnabled(info.id, true);

    if (m_onPluginLoaded) {
        m_onPluginLoaded(info);
    }

    std::cout << "Loaded plugin: " << info.name << " v" << info.version << std::endl;
    return PluginResult::ok();
}

PluginResult PluginManager::unloadPlugin(const UUID& pluginId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        if ((*it)->info.id == pluginId && (*it)->loaded) {
            PluginInterface* instance = (*it)->instance;
            PluginDestroyFunc destroyFunc = (*it)->destroyFunc;
            void* handle = (*it)->handle;
            PluginInfo info = (*it)->info;

            PluginResult shutdownResult = instance->shutdown();
            if (!shutdownResult.success) {
                std::cerr << "Warning: Plugin shutdown failed: " << shutdownResult.errorMessage << std::endl;
            }

            destroyFunc(instance);
            unloadSharedLibrary(handle);

            m_plugins.erase(it);
            m_registry.unregisterPlugin(pluginId);

            if (m_onPluginUnloaded) {
                m_onPluginUnloaded(info);
            }

            std::cout << "Unloaded plugin: " << info.name << std::endl;
            return PluginResult::ok();
        }
    }

    return PluginResult::error("Plugin not found or not loaded: " + pluginId);
}

PluginResult PluginManager::loadAllPlugins() {
    discoverPlugins();

    std::lock_guard<std::mutex> lock(m_mutex);
    PluginResult lastError;

    for (const auto& entry : m_plugins) {
        if (!entry->loaded) {
            PluginResult result = loadPlugin(entry->info.filePath);
            if (!result.success) {
                lastError = result;
            }
        }
    }

    return lastError.success ? PluginResult::ok() : lastError;
}

PluginResult PluginManager::unloadAllPlugins() {
    std::vector<UUID> toUnload;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& entry : m_plugins) {
            if (entry->loaded) {
                toUnload.push_back(entry->info.id);
            }
        }
    }

    PluginResult lastError;
    for (const auto& id : toUnload) {
        PluginResult result = unloadPlugin(id);
        if (!result.success) {
            lastError = result;
        }
    }

    return lastError.success ? PluginResult::ok() : lastError;
}

PluginInterface* PluginManager::getPlugin(const UUID& pluginId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& entry : m_plugins) {
        if (entry->info.id == pluginId && entry->loaded && entry->enabled) {
            return entry->instance;
        }
    }
    return nullptr;
}

PluginInterface* PluginManager::getPluginByName(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& entry : m_plugins) {
        if (entry->info.name == name && entry->loaded && entry->enabled) {
            return entry->instance;
        }
    }
    return nullptr;
}

std::vector<PluginInfo> PluginManager::getLoadedPluginInfos() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PluginInfo> result;
    for (const auto& entry : m_plugins) {
        if (entry->loaded) {
            result.push_back(entry->info);
        }
    }
    return result;
}

std::vector<PluginInfo> PluginManager::getPluginsByType(PluginType type) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<PluginInfo> result;
    for (const auto& entry : m_plugins) {
        if (entry->loaded && entry->info.type == type) {
            result.push_back(entry->info);
        }
    }
    return result;
}

bool PluginManager::isPluginLoaded(const UUID& pluginId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& entry : m_plugins) {
        if (entry->info.id == pluginId && entry->loaded) {
            return true;
        }
    }
    return false;
}

bool PluginManager::isPluginEnabled(const UUID& pluginId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& entry : m_plugins) {
        if (entry->info.id == pluginId) {
            return entry->enabled;
        }
    }
    return false;
}

PluginResult PluginManager::enablePlugin(const UUID& pluginId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& entry : m_plugins) {
        if (entry->info.id == pluginId) {
            entry->enabled = true;
            m_registry.setPluginEnabled(pluginId, true);
            return PluginResult::ok();
        }
    }
    return PluginResult::error("Plugin not found: " + pluginId);
}

PluginResult PluginManager::disablePlugin(const UUID& pluginId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& entry : m_plugins) {
        if (entry->info.id == pluginId) {
            entry->enabled = false;
            m_registry.setPluginEnabled(pluginId, false);
            return PluginResult::ok();
        }
    }
    return PluginResult::error("Plugin not found: " + pluginId);
}

PluginResult PluginManager::validatePluginAPI(const PluginInterface* plugin) const {
    if (!plugin) {
        return PluginResult::error("Plugin instance is null");
    }

    PluginAPIVersion pluginVer = plugin->getAPIVersion();
    PluginAPIVersion supportedMin = PluginAPIVersion::v1_0;
    PluginAPIVersion supportedMax = PluginAPIVersion::v2_0;

    if (static_cast<uint32_t>(pluginVer) < static_cast<uint32_t>(supportedMin) ||
        static_cast<uint32_t>(pluginVer) > static_cast<uint32_t>(supportedMax)) {
        return PluginResult::error("Unsupported plugin API version");
    }

    return PluginResult::ok();
}

UUID PluginManager::generatePluginId(const PluginInfo& info) const {
    std::string combined = info.name + ":" + info.version + ":" + info.filePath;
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (char c : combined) {
        hash ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
        hash *= 0x100000001b3ULL;
    }
    char buf[33];
    snprintf(buf, sizeof(buf), "%016llx%016llx",
             (unsigned long long)(hash >> 32),
             (unsigned long long)(hash & 0xFFFFFFFF));
    return std::string(buf);
}

void* PluginManager::loadSharedLibrary(const std::string& path) {
#ifdef _WIN32
    return static_cast<void*>(LoadLibraryA(path.c_str()));
#else
    return dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
}

void PluginManager::unloadSharedLibrary(void* handle) {
    if (!handle) return;
#ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

PluginCreateFunc PluginManager::getCreateFunction(void* handle) {
#ifdef _WIN32
    return reinterpret_cast<PluginCreateFunc>(GetProcAddress(static_cast<HMODULE>(handle), "createPlugin"));
#else
    return reinterpret_cast<PluginCreateFunc>(dlsym(handle, "createPlugin"));
#endif
}

PluginDestroyFunc PluginManager::getDestroyFunction(void* handle) {
#ifdef _WIN32
    return reinterpret_cast<PluginDestroyFunc>(GetProcAddress(static_cast<HMODULE>(handle), "destroyPlugin"));
#else
    return reinterpret_cast<PluginDestroyFunc>(dlsym(handle, "destroyPlugin"));
#endif
}

} // namespace FreeEffect
