#include "profiler.h"
#include <cstdio>
#include <algorithm>

namespace FreeEffect {

void Profiler::beginFrame() {
    m_frameStart = std::chrono::steady_clock::now();
}

void Profiler::endFrame() {
    auto now = std::chrono::steady_clock::now();
    m_lastFrameTimeMs = std::chrono::duration<double, std::milli>(now - m_frameStart).count();
}

void Profiler::startSection(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentSection = name;
    m_sectionStart = std::chrono::steady_clock::now();
}

void Profiler::endSection(const std::string& name) {
    auto now = std::chrono::steady_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(now - m_sectionStart).count();

    std::lock_guard<std::mutex> lock(m_mutex);
    auto& entry = m_entries[name];
    entry.name = name;
    entry.totalMs += elapsedMs;
    entry.lastMs = elapsedMs;
    entry.callCount++;
    if (elapsedMs < entry.minMs) entry.minMs = elapsedMs;
    if (elapsedMs > entry.maxMs) entry.maxMs = elapsedMs;
    m_currentSection.clear();
}

const std::unordered_map<std::string, ProfileEntry>& Profiler::getEntries() const {
    return m_entries;
}

void Profiler::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries.clear();
    m_lastFrameTimeMs = 0;
}

void Profiler::printReport() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::printf("\n=== FreeEffect Profiler Report ===\n");
    std::printf("Frame time: %.2f ms (%.1f FPS)\n\n", m_lastFrameTimeMs, getFPS());

    struct EntryPair {
        std::string name;
        double totalMs;
    };
    std::vector<EntryPair> sorted;
    for (const auto& kv : m_entries) {
        sorted.push_back({kv.first, kv.second.totalMs});
    }
    std::sort(sorted.begin(), sorted.end(),
        [](const EntryPair& a, const EntryPair& b) { return a.totalMs > b.totalMs; });

    std::printf("%-30s %10s %10s %10s %10s %8s\n",
                "Section", "Total(ms)", "Avg(ms)", "Min(ms)", "Max(ms)", "Calls");
    std::printf("%-30s %10s %10s %10s %10s %8s\n",
                "------------------------------", "----------", "----------",
                "----------", "----------", "--------");

    for (const auto& s : sorted) {
        const auto& entry = m_entries.at(s.name);
        double avgMs = entry.callCount > 0 ? entry.totalMs / entry.callCount : 0;
        std::printf("%-30s %10.2f %10.2f %10.2f %10.2f %8d\n",
                    entry.name.c_str(), entry.totalMs, avgMs,
                    entry.minMs, entry.maxMs, entry.callCount);
    }
    std::printf("===================================\n\n");
}

double Profiler::getFrameTimeMs() const {
    return m_lastFrameTimeMs;
}

double Profiler::getFPS() const {
    if (m_lastFrameTimeMs <= 0.0) return 0.0;
    return 1000.0 / m_lastFrameTimeMs;
}

} // namespace FreeEffect
