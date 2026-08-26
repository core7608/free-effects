#include "importer.h"
#include <algorithm>
#include <filesystem>

namespace FreeEffect {

Importer::Importer(ProjectState* project)
    : m_project(project) {
}

AssetPtr Importer::importFile(const std::string& filePath) {
    if (!std::filesystem::exists(filePath)) {
        return nullptr;
    }
    
    AssetType type = detectFileType(filePath);
    std::string name = getFileName(filePath);
    
    return m_project->addAsset(filePath, name, type);
}

AssetPtr Importer::importImage(const std::string& filePath) {
    if (!isImageFormat(filePath)) return nullptr;
    return importFile(filePath);
}

AssetPtr Importer::importAudio(const std::string& filePath) {
    if (!isAudioFormat(filePath)) return nullptr;
    return importFile(filePath);
}

AssetPtr Importer::importVideo(const std::string& filePath) {
    if (!isVideoFormat(filePath)) return nullptr;
    return importFile(filePath);
}

AssetType Importer::detectFileType(const std::string& filePath) const {
    std::string ext = std::filesystem::path(filePath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (isVideoFormat(ext)) return AssetType::Video;
    if (isImageFormat(ext)) return AssetType::Image;
    if (isAudioFormat(ext)) return AssetType::Audio;
    
    return AssetType::Video; // Default fallback
}

bool Importer::isSupportedFormat(const std::string& filePath) const {
    std::string ext = std::filesystem::path(filePath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    return isVideoFormat(ext) || isImageFormat(ext) || isAudioFormat(ext);
}

bool Importer::isImageFormat(const std::string& ext) {
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || 
           ext == ".bmp" || ext == ".tiff" || ext == ".tif" || ext == ".gif" ||
           ext == ".exr" || ext == ".hdr" || ext == ".rgbe" ||
           ext == ".dpx" || ext == ".tga" || ext == ".vda" || ext == ".icb" || ext == ".vst";
}

bool Importer::isAudioFormat(const std::string& ext) {
    return ext == ".wav" || ext == ".mp3" || ext == ".ogg" || 
           ext == ".flac" || ext == ".aac" || ext == ".m4a";
}

bool Importer::isVideoFormat(const std::string& ext) {
    return ext == ".mp4" || ext == ".mov" || ext == ".avi" || 
           ext == ".mkv" || ext == ".webm" || ext == ".wmv" ||
           ext == ".m4v" || ext == ".mpg" || ext == ".mpeg";
}

std::string Importer::getFileName(const std::string& filePath) const {
    return std::filesystem::path(filePath).filename().string();
}

} // namespace FreeEffect
