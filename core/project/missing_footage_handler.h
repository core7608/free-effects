#pragma once

#include "project_state.h"
#include <functional>
#include <string>
#include <vector>

namespace FreeEffect {

struct MissingFootage {
    UUID assetId;
    std::string expectedPath;
    std::string name;
};

class MissingFootageHandler {
public:
    explicit MissingFootageHandler(ProjectState* project);
    
    std::vector<MissingFootage> checkForMissingFootage() const;
    bool hasMissingFootage() const;
    
    bool relinkAsset(const UUID& assetId, const std::string& newPath);
    int relinkAll(const std::string& oldBasePath, const std::string& newBasePath);
    
    void setAutoCheckEnabled(bool enabled) { m_autoCheck = enabled; }
    bool isAutoCheckEnabled() const { return m_autoCheck; }

private:
    ProjectState* m_project;
    bool m_autoCheck = true;
};

} // namespace FreeEffect
