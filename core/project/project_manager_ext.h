#pragma once
#include <string>
#include <vector>

namespace FreeEffect {

struct OpenProject {
    std::string path;
    std::string name;
    bool modified = false;
    bool active = false;
    double lastAccessTime = 0;
};

class MultiProjectManager {
public:
    void openProject(const std::string& path);
    void closeProject(const std::string& path);
    void setActiveProject(const std::string& path);

    std::vector<OpenProject> getOpenProjects() const { return m_openProjects; }
    std::string getActiveProjectPath() const;

    bool hasUnsavedChanges(const std::string& path) const;
    void markModified(const std::string& path);
    void markSaved(const std::string& path);

    bool copyLayer(const std::string& srcProject, int srcCompIndex, int srcLayerIndex,
                   const std::string& dstProject, int dstCompIndex);
    bool copyComp(const std::string& srcProject, int srcCompIndex,
                  const std::string& dstProject);

    int getOpenProjectCount() const { return static_cast<int>(m_openProjects.size()); }
    bool isProjectOpen(const std::string& path) const;

private:
    std::vector<OpenProject> m_openProjects;

    int findProject(const std::string& path) const;
};

} // namespace FreeEffect
