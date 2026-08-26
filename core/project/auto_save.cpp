#include "auto_save.h"
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace FreeEffect {

AutoSave& AutoSave::instance() {
    static AutoSave s_instance;
    return s_instance;
}

void AutoSave::setEnabled(bool enabled) {
    m_enabled = enabled;
}

bool AutoSave::isEnabled() const {
    return m_enabled;
}

void AutoSave::setInterval(int minutes) {
    m_intervalMinutes = std::max(1, minutes);
}

int AutoSave::getInterval() const {
    return m_intervalMinutes;
}

void AutoSave::setMaxSaves(int max) {
    m_maxSaves = std::max(1, max);
}

int AutoSave::getMaxSaves() const {
    return m_maxSaves;
}

void AutoSave::setDirectory(const std::string& dir) {
    m_directory = dir;
    try {
        std::filesystem::create_directories(dir);
    } catch (...) {
    }
}

void AutoSave::save(const std::string& projectPath) {
    if (!m_enabled || projectPath.empty()) return;

    std::string saveDir = m_directory;
    if (saveDir.empty()) {
        saveDir = std::filesystem::path(projectPath).parent_path().string() + "/.autosave";
    }

    try {
        std::filesystem::create_directories(saveDir);
    } catch (...) {
        return;
    }

    std::string projectName = std::filesystem::path(projectPath).stem().string();
    auto now = std::chrono::steady_clock::now();
    auto timeSinceEpoch = now.time_since_epoch().count();
    std::string autoSaveName = projectName + "_autosave_" + std::to_string(timeSinceEpoch) + ".feproj";
    std::string savePath = saveDir + "/" + autoSaveName;

    try {
        std::filesystem::copy_file(projectPath, savePath,
            std::filesystem::copy_options::overwrite_existing);
    } catch (...) {
        std::ifstream src(projectPath, std::ios::binary);
        std::ofstream dst(savePath, std::ios::binary);
        if (src.is_open() && dst.is_open()) {
            dst << src.rdbuf();
        }
    }

    m_lastSave = now;
    cleanupOldSaves();
}

bool AutoSave::restore(const std::string& autoSavePath) {
    if (!std::filesystem::exists(autoSavePath)) return false;
    return true;
}

std::vector<AutoSaveEntry> AutoSave::getAvailableSaves() const {
    std::vector<AutoSaveEntry> entries;

    std::string saveDir = m_directory;
    if (saveDir.empty()) return entries;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(saveDir)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".feproj") {
                    AutoSaveEntry ase;
                    ase.filePath = entry.path().string();
                    ase.fileSize = entry.file_size();
                    ase.projectName = entry.path().stem().string();

                    auto ftime = std::filesystem::last_write_time(entry.path());
                    ase.savedAt = std::chrono::steady_clock::time_point(
                        std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                            ftime.time_since_epoch()
                        )
                    );

                    entries.push_back(ase);
                }
            }
        }

        std::sort(entries.begin(), entries.end(), [](const AutoSaveEntry& a, const AutoSaveEntry& b) {
            return a.savedAt > b.savedAt;
        });
    } catch (...) {
    }

    return entries;
}

void AutoSave::cleanupOldSaves() {
    auto entries = getAvailableSaves();
    while (static_cast<int>(entries.size()) > m_maxSaves) {
        try {
            std::filesystem::remove(entries.back().filePath);
        } catch (...) {
        }
        entries.pop_back();
    }
}

void AutoSave::update() {
    if (!m_enabled) return;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - m_lastSave).count();

    if (elapsed >= m_intervalMinutes) {
        m_lastSave = now;
    }
}

} // namespace FreeEffect
