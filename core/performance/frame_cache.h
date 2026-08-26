#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <functional>

namespace FreeEffect {

struct CacheEntry {
    std::vector<uint8_t> pixelData;
    int width = 0;
    int height = 0;
    double timestamp = -1.0;
    std::chrono::steady_clock::time_point cachedAt;
    bool dirty = true;

    bool isValid(double maxAgeMs = 1000.0) const {
        if (dirty) return false;
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - cachedAt).count();
        return elapsed < maxAgeMs;
    }

    size_t memoryBytes() const {
        return pixelData.size();
    }
};

class FrameCache {
public:
    static FrameCache& instance() {
        static FrameCache inst;
        return inst;
    }

    CacheEntry& getOrCompute(double time, int compId,
        std::function<void(std::vector<uint8_t>&, int&, int&)> compute);

    void invalidate(double time, int compId);
    void invalidateAll();
    void invalidateRange(double startTime, double endTime, int compId);

    size_t getMemoryUsage() const;
    void setMaxMemoryMB(size_t mb);
    void evictOldest();

    struct Stats {
        size_t hits = 0;
        size_t misses = 0;
        size_t evictions = 0;
        size_t totalMemoryBytes = 0;
    };
    Stats getStats() const;

private:
    FrameCache() = default;
    mutable std::mutex m_mutex;
    std::unordered_map<uint64_t, CacheEntry> m_cache;
    size_t m_maxMemoryBytes = 512 * 1024 * 1024;
    Stats m_stats;

    uint64_t makeKey(double time, int compId) const;
};

} // namespace FreeEffect
