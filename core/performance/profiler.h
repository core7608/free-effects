#pragma once
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace FreeEffect {

struct ProfileEntry {
    std::string name;
    double totalMs = 0;
    double minMs = 1e9;
    double maxMs = 0;
    int callCount = 0;
    double lastMs = 0;
};

class Profiler {
public:
    static Profiler& instance() {
        static Profiler inst;
        return inst;
    }

    void beginFrame();
    void endFrame();

    void startSection(const std::string& name);
    void endSection(const std::string& name);

    const std::unordered_map<std::string, ProfileEntry>& getEntries() const;
    void reset();
    void printReport() const;

    double getFrameTimeMs() const;
    double getFPS() const;

private:
    Profiler() = default;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, ProfileEntry> m_entries;

    std::chrono::steady_clock::time_point m_frameStart;
    std::chrono::steady_clock::time_point m_sectionStart;
    std::string m_currentSection;
    double m_lastFrameTimeMs = 0;
};

class ProfileScope {
public:
    ProfileScope(const std::string& name) : m_name(name) {
        Profiler::instance().startSection(m_name);
    }
    ~ProfileScope() {
        Profiler::instance().endSection(m_name);
    }
private:
    std::string m_name;
};

} // namespace FreeEffect

#define FREEEFFECT_PROFILE_SCOPE(name) FreeEffect::ProfileScope _profileScope##__LINE__(name)
#define FREEEFFECT_PROFILE_FUNCTION() FREEEFFECT_PROFILE_SCOPE(__func__)
