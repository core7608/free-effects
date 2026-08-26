#include "frame_cache.h"
#include <algorithm>
#include <cmath>
#include <functional>

namespace FreeEffect {

uint64_t FrameCache::makeKey(double time, int compId) const {
    // Quantize time to 1ms precision to avoid floating-point key drift
    int64_t timeQuantized = static_cast<int64_t>(std::round(time * 1000.0));
    uint64_t hi = static_cast<uint64_t>(timeQuantized) & 0xFFFFFFFFULL;
    uint64_t lo = static_cast<uint64_t>(static_cast<uint32_t>(compId));
    return (hi << 32) | lo;
}

CacheEntry& FrameCache::getOrCompute(double time, int compId,
    std::function<void(std::vector<uint8_t>&, int&, int&)> compute)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    uint64_t key = makeKey(time, compId);

    auto it = m_cache.find(key);
    if (it != m_cache.end() && it->second.isValid(2000.0)) {
        m_stats.hits++;
        it->second.dirty = false;
        return it->second;
    }

    m_stats.misses++;
    CacheEntry& entry = m_cache[key];
    entry.timestamp = time;
    entry.cachedAt = std::chrono::steady_clock::now();
    entry.dirty = false;

    compute(entry.pixelData, entry.width, entry.height);

    m_stats.totalMemoryBytes = 0;
    for (const auto& kv : m_cache) {
        m_stats.totalMemoryBytes += kv.second.memoryBytes();
    }

    evictOldest();
    return entry;
}

void FrameCache::invalidate(double time, int compId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    uint64_t key = makeKey(time, compId);
    auto it = m_cache.find(key);
    if (it != m_cache.end()) {
        it->second.dirty = true;
    }
}

void FrameCache::invalidateAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& kv : m_cache) {
        kv.second.dirty = true;
    }
}

void FrameCache::invalidateRange(double startTime, double endTime, int compId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& kv : m_cache) {
        uint64_t key = kv.first;
        uint32_t storedCompId = static_cast<uint32_t>(key & 0xFFFFFFFFULL);
        if (static_cast<uint32_t>(compId) != storedCompId) continue;

        int64_t storedTimeQuantized = static_cast<int64_t>(key >> 32);
        double storedTime = static_cast<double>(storedTimeQuantized) / 1000.0;
        if (storedTime >= startTime && storedTime <= endTime) {
            kv.second.dirty = true;
        }
    }
}

size_t FrameCache::getMemoryUsage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t total = 0;
    for (const auto& kv : m_cache) {
        total += kv.second.memoryBytes();
    }
    return total;
}

void FrameCache::setMaxMemoryMB(size_t mb) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_maxMemoryBytes = mb * 1024 * 1024;
    evictOldest();
}

void FrameCache::evictOldest() {
    size_t totalMem = 0;
    for (const auto& kv : m_cache) {
        totalMem += kv.second.memoryBytes();
    }

    while (totalMem > m_maxMemoryBytes && !m_cache.empty()) {
        auto oldestIt = m_cache.begin();
        auto now = std::chrono::steady_clock::now();
        auto oldestAge = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - oldestIt->second.cachedAt).count();

        for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
            auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - it->second.cachedAt).count();
            if (age > oldestAge) {
                oldestAge = age;
                oldestIt = it;
            }
        }

        totalMem -= oldestIt->second.memoryBytes();
        m_stats.evictions++;
        m_cache.erase(oldestIt);
    }

    m_stats.totalMemoryBytes = totalMem;
}

FrameCache::Stats FrameCache::getStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    Stats s = m_stats;
    s.totalMemoryBytes = 0;
    for (const auto& kv : m_cache) {
        s.totalMemoryBytes += kv.second.memoryBytes();
    }
    return s;
}

} // namespace FreeEffect
