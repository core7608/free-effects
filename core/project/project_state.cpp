#include "project_state.h"
#include <algorithm>

namespace FreeEffect {

ProjectState::ProjectState() {
}

AssetPtr ProjectState::addAsset(const std::string& path, const std::string& name, AssetType type) {
    auto asset = std::make_shared<AssetReference>(path, name, type);
    m_assets.push_back(asset);
    m_modified = true;
    return asset;
}

void ProjectState::removeAsset(const UUID& assetId) {
    m_assets.erase(
        std::remove_if(m_assets.begin(), m_assets.end(),
            [&assetId](const AssetPtr& asset) { return asset->getId() == assetId; }),
        m_assets.end());
    m_modified = true;
}

AssetPtr ProjectState::getAssetById(const UUID& assetId) const {
    auto it = std::find_if(m_assets.begin(), m_assets.end(),
        [&assetId](const AssetPtr& asset) { return asset->getId() == assetId; });
    return (it != m_assets.end()) ? *it : nullptr;
}

AssetPtr ProjectState::getAssetByName(const std::string& name) const {
    auto it = std::find_if(m_assets.begin(), m_assets.end(),
        [&name](const AssetPtr& asset) { return asset->getName() == name; });
    return (it != m_assets.end()) ? *it : nullptr;
}

std::shared_ptr<Composition> ProjectState::addComposition(const std::string& name, Resolution res, FrameRate fps, double duration) {
    auto comp = std::make_shared<Composition>(name, res, fps, duration);
    m_compositions.push_back(comp);
    m_modified = true;
    return comp;
}

void ProjectState::removeComposition(const UUID& compId) {
    m_compositions.erase(
        std::remove_if(m_compositions.begin(), m_compositions.end(),
            [&compId](const std::shared_ptr<Composition>& comp) { return comp->getId() == compId; }),
        m_compositions.end());
    m_modified = true;
}

std::shared_ptr<Composition> ProjectState::getCompositionById(const UUID& compId) const {
    auto it = std::find_if(m_compositions.begin(), m_compositions.end(),
        [&compId](const std::shared_ptr<Composition>& comp) { return comp->getId() == compId; });
    return (it != m_compositions.end()) ? *it : nullptr;
}

std::shared_ptr<Composition> ProjectState::getCompositionByName(const std::string& name) const {
    auto it = std::find_if(m_compositions.begin(), m_compositions.end(),
        [&name](const std::shared_ptr<Composition>& comp) { return comp->getName() == name; });
    return (it != m_compositions.end()) ? *it : nullptr;
}

void ProjectState::clear() {
    m_assets.clear();
    m_compositions.clear();
    m_filePath.clear();
    m_modified = false;
}

} // namespace FreeEffect
