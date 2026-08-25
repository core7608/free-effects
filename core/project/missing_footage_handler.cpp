#include "missing_footage_handler.h"
#include <filesystem>

namespace FreeEffect {

MissingFootageHandler::MissingFootageHandler(ProjectState* project)
    : m_project(project) {
}

std::vector<MissingFootage> MissingFootageHandler::checkForMissingFootage() const {
    std::vector<MissingFootage> missing;
    
    for (const auto& asset : m_project->getAssets()) {
        if (asset->getType() == AssetType::Composition) continue;
        
        if (!std::filesystem::exists(asset->getPath())) {
            missing.push_back({asset->getId(), asset->getPath(), asset->getName()});
        }
    }
    
    return missing;
}

bool MissingFootageHandler::hasMissingFootage() const {
    for (const auto& asset : m_project->getAssets()) {
        if (asset->getType() == AssetType::Composition) continue;
        if (!std::filesystem::exists(asset->getPath())) {
            return true;
        }
    }
    return false;
}

bool MissingFootageHandler::relinkAsset(const UUID& assetId, const std::string& newPath) {
    auto asset = m_project->getAssetById(assetId);
    if (!asset) return false;
    
    if (!std::filesystem::exists(newPath)) return false;
    
    asset->setPath(newPath);
    asset->setStatus(AssetStatus::Available);
    return true;
}

int MissingFootageHandler::relinkAll(const std::string& oldBasePath, const std::string& newBasePath) {
    int relinked = 0;
    
    for (const auto& asset : m_project->getAssets()) {
        if (asset->getType() == AssetType::Composition) continue;
        
        const std::string& path = asset->getPath();
        
        // Check if the path starts with the old base path
        if (path.find(oldBasePath) == 0) {
            std::string newPath = newBasePath + path.substr(oldBasePath.length());
            
            if (std::filesystem::exists(newPath)) {
                asset->setPath(newPath);
                asset->setStatus(AssetStatus::Available);
                relinked++;
            }
        }
    }
    
    return relinked;
}

} // namespace FreeEffect
