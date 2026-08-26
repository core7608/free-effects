#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <unordered_map>

namespace FreeEffect {

struct DiskCacheEntry {
    std::string id;
    std::string filePath;
    double time = 0;
    int compId = 0;
    int width = 0, height = 0;
    size_t dataSize = 0;
    std::chrono::steady_clock::time_point cachedAt;
    bool dirty = false;
};

class DiskCache {
public:
    static DiskCache& instance();

    void setCacheDirectory(const std::string& dir);
    std::string getCacheDirectory() const;

    void setMaxSizeMB(size_t mb);
    size_t getMaxSizeMB() const;
    size_t getCurrentSizeMB() const;

    bool storeFrame(double time, int compId, int width, int height, const uint8_t* pixelData, size_t dataSize);
    bool loadFrame(double time, int compId, uint8_t* pixelData, size_t dataSize, int& width, int& height);
    bool hasFrame(double time, int compId) const;
    void invalidate(double time, int compId);
    void invalidateAll();
    void invalidateRange(double startTime, double endTime, int compId);

    void cleanup();
    void compact();

    struct CacheStats {
        size_t totalEntries = 0;
        size_t totalSizeMB = 0;
        size_t hitCount = 0;
        size_t missCount = 0;
        double hitRate = 0;
    };
    CacheStats getStats() const;
    void resetStats();

    void prefetchRange(double startTime, double endTime, int compId, int width, int height);

private:
    DiskCache() = default;

    std::string m_cacheDir;
    size_t m_maxSizeMB = 2048;
    std::vector<DiskCacheEntry> m_entries;
    mutable std::mutex m_mutex;
    CacheStats m_stats;

    std::string getEntryPath(const DiskCacheEntry& entry) const;
    DiskCacheEntry* findEntry(double time, int compId);
    const DiskCacheEntry* findEntry(double time, int compId) const;
    void evictIfNeeded();
    std::string makeIndexPath() const;
    void saveIndex();
    void loadIndex();
    void ensureDirectoryExists();
    void updateHitRate();
};

} // namespace FreeEffect
