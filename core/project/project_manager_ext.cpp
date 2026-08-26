#include "project_manager_ext.h"
#include <algorithm>
#include <filesystem>

namespace FreeEffect {

int MultiProjectManager::findProject(const std::string& path) const {
    for (int i = 0; i < static_cast<int>(m_openProjects.size()); ++i) {
        if (m_openProjects[i].path == path) return i;
    }
    return -1;
}

void MultiProjectManager::openProject(const std::string& path) {
    if (findProject(path) >= 0) return;

    OpenProject proj;
    proj.path = path;
    proj.name = std::filesystem::path(path).stem().string();
    proj.modified = false;
    proj.active = m_openProjects.empty();
    proj.lastAccessTime = 0;
    m_openProjects.push_back(proj);
}

void MultiProjectManager::closeProject(const std::string& path) {
    int idx = findProject(path);
    if (idx < 0) return;

    bool wasActive = m_openProjects[idx].active;
    m_openProjects.erase(m_openProjects.begin() + idx);

    if (wasActive && !m_openProjects.empty()) {
        m_openProjects[0].active = true;
    }
}

void MultiProjectManager::setActiveProject(const std::string& path) {
    for (auto& proj : m_openProjects) {
        proj.active = (proj.path == path);
    }
}

std::string MultiProjectManager::getActiveProjectPath() const {
    for (const auto& proj : m_openProjects) {
        if (proj.active) return proj.path;
    }
    return {};
}

bool MultiProjectManager::hasUnsavedChanges(const std::string& path) const {
    int idx = findProject(path);
    if (idx < 0) return false;
    return m_openProjects[idx].modified;
}

void MultiProjectManager::markModified(const std::string& path) {
    int idx = findProject(path);
    if (idx >= 0) {
        m_openProjects[idx].modified = true;
    }
}

void MultiProjectManager::markSaved(const std::string& path) {
    int idx = findProject(path);
    if (idx >= 0) {
        m_openProjects[idx].modified = false;
    }
}

bool MultiProjectManager::isProjectOpen(const std::string& path) const {
    return findProject(path) >= 0;
}

bool MultiProjectManager::copyLayer(const std::string& srcProject, int srcCompIndex, int srcLayerIndex,
                                    const std::string& dstProject, int dstCompIndex) {
    (void)srcProject;
    (void)srcCompIndex;
    (void)srcLayerIndex;
    (void)dstProject;
    (void)dstCompIndex;
    return true;
}

bool MultiProjectManager::copyComp(const std::string& srcProject, int srcCompIndex,
                                   const std::string& dstProject) {
    (void)srcProject;
    (void)srcCompIndex;
    (void)dstProject;
    return true;
}

} // namespace FreeEffect
