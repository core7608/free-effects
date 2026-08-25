#pragma once

#include "asset_reference.h"
#include "../timeline/composition.h"
#include <memory>
#include <string>
#include <vector>

namespace FreeEffect {

struct ProjectSettings {
    double frameRateBase = 30.0;
    int undoLevels = 32;
};

class ProjectState {
public:
    ProjectState();
    ~ProjectState() = default;
    
    const std::string& getVersion() const { return m_version; }
    
    const std::string& getFilePath() const { return m_filePath; }
    void setFilePath(const std::string& path) { m_filePath = path; }
    
    bool isModified() const { return m_modified; }
    void setModified(bool modified) { m_modified = modified; }
    
    // Settings
    ProjectSettings& getSettings() { return m_settings; }
    const ProjectSettings& getSettings() const { return m_settings; }
    
    // Assets
    AssetPtr addAsset(const std::string& path, const std::string& name, AssetType type);
    void removeAsset(const UUID& assetId);
    AssetPtr getAssetById(const UUID& assetId) const;
    AssetPtr getAssetByName(const std::string& name) const;
    const std::vector<AssetPtr>& getAssets() const { return m_assets; }
    
    // Compositions
    std::shared_ptr<Composition> addComposition(const std::string& name, Resolution res, FrameRate fps, double duration);
    void removeComposition(const UUID& compId);
    std::shared_ptr<Composition> getCompositionById(const UUID& compId) const;
    std::shared_ptr<Composition> getCompositionByName(const std::string& name) const;
    const std::vector<std::shared_ptr<Composition>>& getCompositions() const { return m_compositions; }
    
    void clear();

private:
    std::string m_version = "0.1.0";
    std::string m_filePath;
    bool m_modified = false;
    ProjectSettings m_settings;
    std::vector<AssetPtr> m_assets;
    std::vector<std::shared_ptr<Composition>> m_compositions;
};

} // namespace FreeEffect
