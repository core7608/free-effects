#include "team_project.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <random>

namespace FreeEffect {

std::string TeamProject::generateVersionId() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);
    char buf[33];
    snprintf(buf, sizeof(buf), "%08x%08x%08x%08x",
             dis(gen), dis(gen), dis(gen), dis(gen));
    return std::string(buf);
}

bool TeamProject::saveVersion(const std::string& projectPath, const std::string& author, const std::string& message) {
    ProjectVersion ver;
    ver.id = generateVersionId();
    ver.author = author;
    ver.message = message;
    ver.createdAt = std::chrono::system_clock::now();
    ver.filePath = projectPath;

    std::ifstream file(projectPath, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        ver.fileSize = static_cast<size_t>(file.tellg());
    }

    m_versions.push_back(ver);

    std::string backupPath = projectPath + ".v" + ver.id;
    std::ifstream src(projectPath, std::ios::binary);
    if (src.is_open()) {
        std::ofstream dst(backupPath, std::ios::binary);
        if (dst.is_open()) {
            dst << src.rdbuf();
        }
    }

    return true;
}

bool TeamProject::restoreVersion(const std::string& versionId) {
    auto it = std::find_if(m_versions.begin(), m_versions.end(),
        [&versionId](const ProjectVersion& v) { return v.id == versionId; });
    if (it == m_versions.end()) return false;

    std::string backupPath = it->filePath + ".v" + versionId;
    std::ifstream src(backupPath, std::ios::binary);
    if (!src.is_open()) return false;

    std::ofstream dst(it->filePath, std::ios::binary | std::ios::trunc);
    if (!dst.is_open()) return false;

    dst << src.rdbuf();
    return dst.good();
}

std::vector<ProjectVersion> TeamProject::getVersionHistory(const std::string& projectPath) const {
    std::vector<ProjectVersion> result;
    std::copy_if(m_versions.begin(), m_versions.end(), std::back_inserter(result),
        [&projectPath](const ProjectVersion& v) { return v.filePath == projectPath; });
    std::sort(result.begin(), result.end(),
        [](const ProjectVersion& a, const ProjectVersion& b) { return a.createdAt > b.createdAt; });
    return result;
}

ProjectVersion TeamProject::getVersion(const std::string& versionId) {
    auto it = std::find_if(m_versions.begin(), m_versions.end(),
        [&versionId](const ProjectVersion& v) { return v.id == versionId; });
    if (it != m_versions.end()) return *it;
    return {};
}

bool TeamProject::compareVersions(const std::string& id1, const std::string& id2, std::string& diffReport) {
    auto v1 = getVersion(id1);
    auto v2 = getVersion(id2);

    std::ostringstream oss;
    oss << "Version comparison:\n";
    oss << "  " << id1 << " (" << v1.author << "): " << v1.message << "\n";
    oss << "  " << id2 << " (" << v2.author << "): " << v2.message << "\n";
    oss << "  Size difference: " << static_cast<int64_t>(v2.fileSize) - static_cast<int64_t>(v1.fileSize) << " bytes\n";

    auto t1 = std::chrono::system_clock::to_time_t(v1.createdAt);
    auto t2 = std::chrono::system_clock::to_time_t(v2.createdAt);
    oss << "  Time difference: " << std::difftime(t2, t1) << " seconds\n";

    diffReport = oss.str();
    return true;
}

bool TeamProject::syncToCloud(const std::string& projectPath) {
    if (m_syncCallback) m_syncCallback("Syncing to cloud...", 0.0);

    std::ifstream src(projectPath, std::ios::binary);
    if (!src.is_open()) return false;

    std::string cloudPath = projectPath + ".cloud_sync";
    std::ofstream dst(cloudPath, std::ios::binary);
    if (!dst.is_open()) return false;

    dst << src.rdbuf();
    bool ok = dst.good();

    if (ok) {
        m_syncTimes[projectPath] = std::chrono::system_clock::now();
    }

    if (m_syncCallback) m_syncCallback(ok ? "Sync complete" : "Sync failed", 1.0);
    return ok;
}

bool TeamProject::syncFromCloud(const std::string& projectPath) {
    if (m_syncCallback) m_syncCallback("Syncing from cloud...", 0.0);

    std::string cloudPath = projectPath + ".cloud_sync";
    std::ifstream src(cloudPath, std::ios::binary);
    if (!src.is_open()) return false;

    std::ofstream dst(projectPath, std::ios::binary | std::ios::trunc);
    if (!dst.is_open()) return false;

    dst << src.rdbuf();
    bool ok = dst.good();

    if (ok) {
        m_syncTimes[projectPath] = std::chrono::system_clock::now();
    }

    if (m_syncCallback) m_syncCallback(ok ? "Sync complete" : "Sync failed", 1.0);
    return ok;
}

bool TeamProject::isCloudSynced(const std::string& projectPath) const {
    return m_syncTimes.count(projectPath) > 0;
}

std::chrono::system_clock::time_point TeamProject::getLastSyncTime(const std::string& projectPath) const {
    auto it = m_syncTimes.find(projectPath);
    if (it != m_syncTimes.end()) return it->second;
    return std::chrono::system_clock::time_point{};
}

void TeamProject::addTeamMember(const TeamMember& member) {
    auto it = std::find_if(m_members.begin(), m_members.end(),
        [&member](const TeamMember& m) { return m.email == member.email; });
    if (it == m_members.end()) {
        m_members.push_back(member);
    } else {
        *it = member;
    }
}

void TeamProject::removeTeamMember(const std::string& email) {
    m_members.erase(
        std::remove_if(m_members.begin(), m_members.end(),
            [&email](const TeamMember& m) { return m.email == email; }),
        m_members.end());
}

std::vector<TeamMember> TeamProject::getTeamMembers() const {
    return m_members;
}

std::vector<TeamMember> TeamProject::getOnlineMembers() const {
    std::vector<TeamMember> online;
    std::copy_if(m_members.begin(), m_members.end(), std::back_inserter(online),
        [](const TeamMember& m) { return m.online; });
    return online;
}

bool TeamProject::addComment(const ReviewComment& comment) {
    ReviewComment c = comment;
    if (c.id.empty()) {
        c.id = generateVersionId();
    }
    if (c.createdAt == std::chrono::system_clock::time_point{}) {
        c.createdAt = std::chrono::system_clock::now();
    }
    m_comments.push_back(c);
    return true;
}

bool TeamProject::resolveComment(const std::string& commentId) {
    auto it = std::find_if(m_comments.begin(), m_comments.end(),
        [&commentId](const ReviewComment& c) { return c.id == commentId; });
    if (it == m_comments.end()) return false;
    it->resolved = true;
    return true;
}

std::vector<TeamProject::ReviewComment> TeamProject::getComments(const std::string& projectPath) const {
    std::vector<ReviewComment> result;
    std::copy_if(m_comments.begin(), m_comments.end(), std::back_inserter(result),
        [&projectPath](const ReviewComment& c) {
            (void)projectPath;
            return true;
        });
    return result;
}

void TeamProject::addLabel(const std::string& name, int color) {
    auto it = std::find_if(m_labels.begin(), m_labels.end(),
        [&name](const std::pair<std::string, int>& l) { return l.first == name; });
    if (it == m_labels.end()) {
        m_labels.emplace_back(name, color);
    } else {
        it->second = color;
    }
}

void TeamProject::removeLabel(const std::string& name) {
    m_labels.erase(
        std::remove_if(m_labels.begin(), m_labels.end(),
            [&name](const std::pair<std::string, int>& l) { return l.first == name; }),
        m_labels.end());

    for (auto& [path, labels] : m_itemLabels) {
        labels.erase(
            std::remove(labels.begin(), labels.end(), name),
            labels.end());
    }
}

void TeamProject::assignLabel(const std::string& itemPath, const std::string& labelName) {
    auto& labels = m_itemLabels[itemPath];
    if (std::find(labels.begin(), labels.end(), labelName) == labels.end()) {
        labels.push_back(labelName);
    }
}

std::vector<std::string> TeamProject::getLabels(const std::string& itemPath) const {
    auto it = m_itemLabels.find(itemPath);
    if (it != m_itemLabels.end()) return it->second;
    return {};
}

} // namespace FreeEffect
