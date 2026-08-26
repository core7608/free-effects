#include "project_manager.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cstring>

namespace FreeEffect {

bool ProjectManager::collectFiles(const std::string& projectPath, const std::string& destDir,
                                   bool copyFootage, bool renderAllComps) {
    if (!std::filesystem::exists(projectPath)) return false;

    try {
        std::filesystem::create_directories(destDir);
    } catch (...) {
        return false;
    }

    try {
        std::string projDest = destDir + "/" + std::filesystem::path(projectPath).filename().string();
        std::filesystem::copy_file(projectPath, projDest,
            std::filesystem::copy_options::overwrite_existing);
    } catch (...) {
        std::ifstream src(projectPath, std::ios::binary);
        std::ofstream dst(destDir + "/" + std::filesystem::path(projectPath).filename().string(), std::ios::binary);
        if (src.is_open() && dst.is_open()) dst << src.rdbuf();
    }

    if (copyFootage) {
        auto deps = getDependencies(projectPath);
        std::string footageDir = destDir + "/footage";
        try {
            std::filesystem::create_directories(footageDir);
        } catch (...) {
            return false;
        }

        for (const auto& dep : deps) {
            if (!dep.missing && std::filesystem::exists(dep.resolvedPath)) {
                try {
                    std::string dest = footageDir + "/" + std::filesystem::path(dep.originalPath).filename().string();
                    std::filesystem::copy_file(dep.resolvedPath, dest,
                        std::filesystem::copy_options::overwrite_existing);
                } catch (...) {
                }
            }
        }
    }

    return true;
}

std::vector<AssetDependency> ProjectManager::getDependencies(const std::string& projectPath) {
    std::vector<AssetDependency> deps;

    if (!std::filesystem::exists(projectPath)) return deps;

    std::ifstream file(projectPath, std::ios::binary);
    if (!file.is_open()) return deps;

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    std::string projDir = std::filesystem::path(projectPath).parent_path().string();

    size_t pos = 0;
    while (pos < content.size()) {
        size_t pathStart = content.find("\"path\":", pos);
        if (pathStart == std::string::npos) break;
        pathStart = content.find('"', pathStart + 7);
        if (pathStart == std::string::npos) break;
        pathStart++;
        size_t pathEnd = content.find('"', pathStart);
        if (pathEnd == std::string::npos) break;

        std::string assetPath = content.substr(pathStart, pathEnd - pathStart);
        pos = pathEnd + 1;

        if (assetPath.size() < 2 || assetPath[0] == '{') continue;

        AssetDependency dep;
        dep.originalPath = assetPath;

        if (std::filesystem::exists(assetPath)) {
            dep.resolvedPath = assetPath;
            dep.missing = false;
            try {
                dep.fileSize = std::filesystem::file_size(assetPath);
            } catch (...) {
                dep.fileSize = 0;
            }
        } else {
            std::string localPath = projDir + "/" + assetPath;
            if (std::filesystem::exists(localPath)) {
                dep.resolvedPath = localPath;
                dep.missing = false;
                try {
                    dep.fileSize = std::filesystem::file_size(localPath);
                } catch (...) {
                    dep.fileSize = 0;
                }
            } else {
                dep.resolvedPath = "";
                dep.missing = true;
            }
        }

        deps.push_back(dep);
    }

    return deps;
}

bool ProjectManager::verifyDependencies(const std::string& projectPath) {
    auto deps = getDependencies(projectPath);
    for (const auto& dep : deps) {
        if (dep.missing) return false;
    }
    return true;
}

bool ProjectManager::relinkFootage(const std::string& projectPath, const std::string& oldPath,
                                    const std::string& newPath) {
    if (!std::filesystem::exists(projectPath)) return false;

    std::ifstream fileIn(projectPath, std::ios::binary);
    if (!fileIn.is_open()) return false;

    std::string content((std::istreambuf_iterator<char>(fileIn)),
                         std::istreambuf_iterator<char>());
    fileIn.close();

    size_t pos = 0;
    while ((pos = content.find(oldPath, pos)) != std::string::npos) {
        content.replace(pos, oldPath.length(), newPath);
        pos += newPath.length();
    }

    std::ofstream fileOut(projectPath, std::ios::binary | std::ios::trunc);
    if (!fileOut.is_open()) return false;
    fileOut << content;
    fileOut.close();

    return true;
}

bool ProjectManager::setFootageInterpretation(const std::string& path, const FootageInterpretation& interp) {
    for (auto& existing : m_interpretations) {
        if (existing.path == path) {
            existing = interp;
            existing.path = path;
            return true;
        }
    }
    m_interpretations.push_back(interp);
    return true;
}

ProjectManager::FootageInterpretation ProjectManager::getFootageInterpretation(const std::string& path) const {
    for (const auto& interp : m_interpretations) {
        if (interp.path == path) return interp;
    }
    FootageInterpretation defaultInterp;
    defaultInterp.path = path;
    return defaultInterp;
}

bool ProjectManager::createProxy(const std::string& sourcePath, const std::string& proxyPath, int scalePercent) {
    if (!std::filesystem::exists(sourcePath)) return false;
    if (scalePercent <= 0 || scalePercent > 100) return false;

    std::ifstream src(sourcePath, std::ios::binary);
    std::ofstream dst(proxyPath, std::ios::binary);
    if (!src.is_open() || !dst.is_open()) return false;

    dst << src.rdbuf();
    return true;
}

ProjectManager::ProjectStats ProjectManager::getStats(const std::string& projectPath) {
    ProjectStats stats;

    if (!std::filesystem::exists(projectPath)) return stats;

    try {
        stats.totalAssetSize = std::filesystem::file_size(projectPath);
    } catch (...) {
    }

    std::ifstream file(projectPath, std::ios::binary);
    if (!file.is_open()) return stats;

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    auto countOccurrences = [&content](const std::string& pattern) -> int {
        int count = 0;
        size_t pos = 0;
        while ((pos = content.find(pattern, pos)) != std::string::npos) {
            count++;
            pos += pattern.size();
        }
        return count;
    };

    stats.totalComps = countOccurrences("\"Composition\"");
    stats.totalLayers = countOccurrences("\"Layer\"");
    stats.totalEffects = countOccurrences("\"Effect\"");
    stats.totalKeyframes = countOccurrences("\"Keyframe\"");
    stats.totalAssets = countOccurrences("\"Asset\"");
    stats.totalDuration = 30.0;

    return stats;
}

} // namespace FreeEffect
