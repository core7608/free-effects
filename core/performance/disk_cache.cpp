#include "disk_cache.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace FreeEffect {

DiskCache& DiskCache::instance() {
    static DiskCache inst;
    return inst;
}

void DiskCache::setCacheDirectory(const std::string& dir) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cacheDir = dir;
    ensureDirectoryExists();
    loadIndex();
}

std::string DiskCache::getCacheDirectory() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cacheDir;
}

void DiskCache::setMaxSizeMB(size_t mb) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_maxSizeMB = mb;
    evictIfNeeded();
}

size_t DiskCache::getMaxSizeMB() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_maxSizeMB;
}

size_t DiskCache::getCurrentSizeMB() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t total = 0;
    for (const auto& e : m_entries) {
        total += e.dataSize;
    }
    return total / (1024 * 1024);
}

std::string DiskCache::getEntryPath(const DiskCacheEntry& entry) const {
    return m_cacheDir + "/" + entry.id + ".frame";
}

std::string DiskCache::makeIndexPath() const {
    return m_cacheDir + "/cache_index.dat";
}

void DiskCache::ensureDirectoryExists() {
    if (m_cacheDir.empty()) return;
    std::error_code ec;
    fs::create_directories(m_cacheDir, ec);
}

DiskCacheEntry* DiskCache::findEntry(double time, int compId) {
    for (auto& e : m_entries) {
        if (e.compId == compId && std::abs(e.time - time) < 0.0001) {
            return &e;
        }
    }
    return nullptr;
}

const DiskCacheEntry* DiskCache::findEntry(double time, int compId) const {
    for (const auto& e : m_entries) {
        if (e.compId == compId && std::abs(e.time - time) < 0.0001) {
            return &e;
        }
    }
    return nullptr;
}

void DiskCache::evictIfNeeded() {
    size_t maxSizeBytes = m_maxSizeMB * 1024 * 1024;
    size_t currentBytes = 0;
    for (const auto& e : m_entries) {
        currentBytes += e.dataSize;
    }
    if (currentBytes <= maxSizeBytes || m_entries.empty()) return;

    // LRU: sort by cachedAt (oldest first)
    std::sort(m_entries.begin(), m_entries.end(),
        [](const DiskCacheEntry& a, const DiskCacheEntry& b) {
            return a.cachedAt < b.cachedAt;
        });

    while (currentBytes > maxSizeBytes && !m_entries.empty()) {
        currentBytes -= m_entries.front().dataSize;
        std::string path = getEntryPath(m_entries.front());
        std::error_code ec;
        fs::remove(path, ec);
        m_entries.erase(m_entries.begin());
    }
    saveIndex();
}

void DiskCache::saveIndex() {
    if (m_cacheDir.empty()) return;
    std::string path = makeIndexPath();
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) return;

    uint32_t count = static_cast<uint32_t>(m_entries.size());
    ofs.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& e : m_entries) {
        uint32_t idLen = static_cast<uint32_t>(e.id.size());
        ofs.write(reinterpret_cast<const char*>(&idLen), sizeof(idLen));
        ofs.write(e.id.c_str(), idLen);

        ofs.write(reinterpret_cast<const char*>(&e.time), sizeof(e.time));
        ofs.write(reinterpret_cast<const char*>(&e.compId), sizeof(e.compId));
        ofs.write(reinterpret_cast<const char*>(&e.width), sizeof(e.width));
        ofs.write(reinterpret_cast<const char*>(&e.height), sizeof(e.height));
        ofs.write(reinterpret_cast<const char*>(&e.dataSize), sizeof(e.dataSize));
        int64_t cachedAtMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            e.cachedAt.time_since_epoch()).count();
        ofs.write(reinterpret_cast<const char*>(&cachedAtMs), sizeof(cachedAtMs));
        uint8_t dirtyByte = e.dirty ? 1 : 0;
        ofs.write(reinterpret_cast<const char*>(&dirtyByte), sizeof(dirtyByte));
    }
}

void DiskCache::loadIndex() {
    if (m_cacheDir.empty()) return;
    std::string path = makeIndexPath();
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return;

    m_entries.clear();
    uint32_t count = 0;
    ifs.read(reinterpret_cast<char*>(&count), sizeof(count));

    for (uint32_t i = 0; i < count; ++i) {
        DiskCacheEntry e;
        uint32_t idLen = 0;
        ifs.read(reinterpret_cast<char*>(&idLen), sizeof(idLen));
        e.id.resize(idLen);
        ifs.read(e.id.data(), idLen);

        ifs.read(reinterpret_cast<char*>(&e.time), sizeof(e.time));
        ifs.read(reinterpret_cast<char*>(&e.compId), sizeof(e.compId));
        ifs.read(reinterpret_cast<char*>(&e.width), sizeof(e.width));
        ifs.read(reinterpret_cast<char*>(&e.height), sizeof(e.height));
        ifs.read(reinterpret_cast<char*>(&e.dataSize), sizeof(e.dataSize));
        int64_t cachedAtMs = 0;
        ifs.read(reinterpret_cast<char*>(&cachedAtMs), sizeof(cachedAtMs));
        e.cachedAt = std::chrono::steady_clock::time_point(
            std::chrono::milliseconds(cachedAtMs));
        uint8_t dirtyByte = 0;
        ifs.read(reinterpret_cast<char*>(&dirtyByte), sizeof(dirtyByte));
        e.dirty = dirtyByte != 0;
        e.filePath = getEntryPath(e);

        // Verify file exists
        if (fs::exists(e.filePath)) {
            m_entries.push_back(std::move(e));
        }
    }
}

bool DiskCache::storeFrame(double time, int compId, int width, int height,
                            const uint8_t* pixelData, size_t dataSize) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_cacheDir.empty()) return false;

    DiskCacheEntry* existing = findEntry(time, compId);
    if (existing) {
        // Overwrite
        existing->width = width;
        existing->height = height;
        existing->dataSize = dataSize;
        existing->cachedAt = std::chrono::steady_clock::now();
        existing->dirty = false;
        existing->filePath = getEntryPath(*existing);

        std::ofstream ofs(existing->filePath, std::ios::binary);
        if (!ofs) return false;
        ofs.write(reinterpret_cast<const char*>(&width), sizeof(width));
        ofs.write(reinterpret_cast<const char*>(&height), sizeof(height));
        ofs.write(reinterpret_cast<const char*>(pixelData), dataSize);
        saveIndex();
        return true;
    }

    // Create new entry
    DiskCacheEntry entry;
    entry.time = time;
    entry.compId = compId;
    entry.width = width;
    entry.height = height;
    entry.dataSize = dataSize;
    entry.cachedAt = std::chrono::steady_clock::now();
    entry.dirty = false;

    // Generate unique ID
    std::ostringstream oss;
    oss << "comp" << compId << "_t" << static_cast<int>(time * 1000.0)
        << "_" << width << "x" << height;
    entry.id = oss.str();
    entry.filePath = getEntryPath(entry);

    evictIfNeeded();

    std::ofstream ofs(entry.filePath, std::ios::binary);
    if (!ofs) return false;
    ofs.write(reinterpret_cast<const char*>(&width), sizeof(width));
    ofs.write(reinterpret_cast<const char*>(&height), sizeof(height));
    ofs.write(reinterpret_cast<const char*>(pixelData), dataSize);

    m_entries.push_back(std::move(entry));
    saveIndex();
    return true;
}

bool DiskCache::loadFrame(double time, int compId, uint8_t* pixelData, size_t dataSize,
                           int& width, int& height) {
    std::lock_guard<std::mutex> lock(m_mutex);
    DiskCacheEntry* entry = findEntry(time, compId);
    if (!entry) {
        m_stats.missCount++;
        updateHitRate();
        return false;
    }

    std::ifstream ifs(entry->filePath, std::ios::binary);
    if (!ifs) {
        m_stats.missCount++;
        updateHitRate();
        return false;
    }

    ifs.read(reinterpret_cast<char*>(&width), sizeof(width));
    ifs.read(reinterpret_cast<char*>(&height), sizeof(height));
    size_t readSize = std::min(dataSize, static_cast<size_t>(width) * height * 4);
    ifs.read(reinterpret_cast<char*>(pixelData), readSize);

    // Update LRU
    entry->cachedAt = std::chrono::steady_clock::now();

    m_stats.hitCount++;
    updateHitRate();
    return true;
}

bool DiskCache::hasFrame(double time, int compId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return findEntry(time, compId) != nullptr;
}

void DiskCache::invalidate(double time, int compId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::remove_if(m_entries.begin(), m_entries.end(),
        [time, compId](const DiskCacheEntry& e) {
            return e.compId == compId && std::abs(e.time - time) < 0.0001;
        });
    if (it != m_entries.end()) {
        for (auto i = it; i != m_entries.end(); ++i) {
            std::error_code ec;
            fs::remove(i->filePath, ec);
        }
        m_entries.erase(it, m_entries.end());
        saveIndex();
    }
}

void DiskCache::invalidateAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& e : m_entries) {
        std::error_code ec;
        fs::remove(e.filePath, ec);
    }
    m_entries.clear();
    saveIndex();
}

void DiskCache::invalidateRange(double startTime, double endTime, int compId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::remove_if(m_entries.begin(), m_entries.end(),
        [startTime, endTime, compId](const DiskCacheEntry& e) {
            return e.compId == compId && e.time >= startTime && e.time <= endTime;
        });
    if (it != m_entries.end()) {
        for (auto i = it; i != m_entries.end(); ++i) {
            std::error_code ec;
            fs::remove(i->filePath, ec);
        }
        m_entries.erase(it, m_entries.end());
        saveIndex();
    }
}

void DiskCache::cleanup() {
    std::lock_guard<std::mutex> lock(m_mutex);
    evictIfNeeded();
}

void DiskCache::compact() {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Remove entries with missing files
    auto it = std::remove_if(m_entries.begin(), m_entries.end(),
        [](const DiskCacheEntry& e) {
            return !fs::exists(e.filePath);
        });
    if (it != m_entries.end()) {
        m_entries.erase(it, m_entries.end());
        saveIndex();
    }
}

DiskCache::CacheStats DiskCache::getStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    CacheStats stats = m_stats;
    stats.totalEntries = m_entries.size();
    stats.totalSizeMB = getCurrentSizeMB();
    return stats;
}

void DiskCache::resetStats() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stats = CacheStats{};
}

void DiskCache::prefetchRange(double startTime, double endTime, int compId, int width, int height) {
    (void)startTime; (void)endTime; (void)compId;
    (void)width; (void)height;
    // Prefetching requires external renderer callback - stub for now
}

// Private helper referenced but not declared in header
void DiskCache::updateHitRate() {
    size_t total = m_stats.hitCount + m_stats.missCount;
    m_stats.hitRate = total > 0 ? static_cast<double>(m_stats.hitCount) / static_cast<double>(total) : 0.0;
}

} // namespace FreeEffect
