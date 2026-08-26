#pragma once

#include <string>
#include <vector>

namespace FreeEffect {

struct AssetDependency {
    std::string originalPath;
    std::string resolvedPath;
    bool missing = false;
    size_t fileSize = 0;
};

class ProjectManager {
public:
    bool collectFiles(const std::string& projectPath, const std::string& destDir,
                      bool copyFootage = true, bool renderAllComps = false);

    std::vector<AssetDependency> getDependencies(const std::string& projectPath);

    bool verifyDependencies(const std::string& projectPath);

    bool relinkFootage(const std::string& projectPath, const std::string& oldPath, const std::string& newPath);

    struct FootageInterpretation {
        std::string path;
        double frameRate = 30.0;
        double startTime = 0;
        int alphaMode = 0;
        double loopCount = 1;
        int fieldOrder = 0;
        bool premultiply = true;
    };

    bool setFootageInterpretation(const std::string& path, const FootageInterpretation& interp);
    FootageInterpretation getFootageInterpretation(const std::string& path) const;

    bool createProxy(const std::string& sourcePath, const std::string& proxyPath, int scalePercent);

    struct ProjectStats {
        int totalComps = 0;
        int totalLayers = 0;
        int totalEffects = 0;
        int totalKeyframes = 0;
        int totalAssets = 0;
        size_t totalAssetSize = 0;
        double totalDuration = 0;
    };

    ProjectStats getStats(const std::string& projectPath);

private:
    std::vector<FootageInterpretation> m_interpretations;
};

} // namespace FreeEffect
