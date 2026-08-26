#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace FreeEffect {

struct AutoSaveEntry {
    std::string filePath;
    std::chrono::steady_clock::time_point savedAt;
    size_t fileSize = 0;
    std::string projectName;
};

class AutoSave {
public:
    static AutoSave& instance();

    void setEnabled(bool enabled);
    bool isEnabled() const;

    void setInterval(int minutes);
    int getInterval() const;

    void setMaxSaves(int max);
    int getMaxSaves() const;

    void setDirectory(const std::string& dir);

    void save(const std::string& projectPath);
    bool restore(const std::string& autoSavePath);

    std::vector<AutoSaveEntry> getAvailableSaves() const;
    void cleanupOldSaves();

    void update();

private:
    AutoSave() = default;

    bool m_enabled = true;
    int m_intervalMinutes = 5;
    int m_maxSaves = 10;
    std::string m_directory;
    std::chrono::steady_clock::time_point m_lastSave;
};

} // namespace FreeEffect
