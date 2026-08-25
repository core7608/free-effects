#pragma once

#include "../project/asset_reference.h"
#include "../project/project_state.h"
#include <string>

namespace FreeEffect {

class Importer {
public:
    Importer(ProjectState* project);
    ~Importer() = default;
    
    AssetPtr importFile(const std::string& filePath);
    AssetPtr importImage(const std::string& filePath);
    AssetPtr importAudio(const std::string& filePath);
    AssetPtr importVideo(const std::string& filePath);
    
    AssetType detectFileType(const std::string& filePath) const;
    bool isSupportedFormat(const std::string& filePath) const;

private:
    ProjectState* m_project;
    
    static bool isImageFormat(const std::string& ext);
    static bool isAudioFormat(const std::string& ext);
    static bool isVideoFormat(const std::string& ext);
    std::string getFileName(const std::string& filePath) const;
};

} // namespace FreeEffect
