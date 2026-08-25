#pragma once

#include "../project/project_state.h"
#include <string>

namespace FreeEffect {

struct ProjectLoadResult {
    bool success = false;
    std::string errorMessage;
    std::string filePath;
};

class ProjectFile {
public:
    ProjectFile() = default;
    ~ProjectFile() = default;
    
    bool save(const ProjectState& project, const std::string& filePath);
    ProjectLoadResult load(const std::string& filePath, ProjectState& project);
    
    bool saveToCurrentPath(const ProjectState& project);
    
    static std::string getVersion() { return "0.1.0"; }

private:
    void writeAssets(const ProjectState& project, /* json& */ void* json);
    void writeCompositions(const ProjectState& project, /* json& */ void* json);
    void readAssets(/* json& */ void* json, ProjectState& project);
    void readCompositions(/* json& */ void* json, ProjectState& project);
};

} // namespace FreeEffect
