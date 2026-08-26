#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <functional>

namespace FreeEffect {

struct ProjectVersion {
    std::string id;
    std::string author;
    std::string message;
    std::chrono::system_clock::time_point createdAt;
    size_t fileSize = 0;
    std::string filePath;
};

struct TeamMember {
    std::string name;
    std::string email;
    std::string avatar;
    bool online = false;
    std::string currentFile;
};

class TeamProject {
public:
    // Version control
    bool saveVersion(const std::string& projectPath, const std::string& author, const std::string& message);
    bool restoreVersion(const std::string& versionId);
    std::vector<ProjectVersion> getVersionHistory(const std::string& projectPath) const;
    ProjectVersion getVersion(const std::string& versionId);
    bool compareVersions(const std::string& id1, const std::string& id2, std::string& diffReport);

    // Cloud sync
    bool syncToCloud(const std::string& projectPath);
    bool syncFromCloud(const std::string& projectPath);
    bool isCloudSynced(const std::string& projectPath) const;
    std::chrono::system_clock::time_point getLastSyncTime(const std::string& projectPath) const;

    // Team members
    void addTeamMember(const TeamMember& member);
    void removeTeamMember(const std::string& email);
    std::vector<TeamMember> getTeamMembers() const;
    std::vector<TeamMember> getOnlineMembers() const;

    // Review workflow
    struct ReviewComment {
        std::string id;
        std::string author;
        std::string text;
        double time = 0;
        int compIndex = 0;
        std::chrono::system_clock::time_point createdAt;
        bool resolved = false;
    };
    bool addComment(const ReviewComment& comment);
    bool resolveComment(const std::string& commentId);
    std::vector<ReviewComment> getComments(const std::string& projectPath) const;

    // Labels
    void addLabel(const std::string& name, int color);
    void removeLabel(const std::string& name);
    void assignLabel(const std::string& itemPath, const std::string& labelName);
    std::vector<std::string> getLabels(const std::string& itemPath) const;

    // Status callbacks
    using SyncCallback = std::function<void(const std::string& status, double progress)>;
    void setSyncCallback(SyncCallback cb) { m_syncCallback = std::move(cb); }

private:
    std::vector<ProjectVersion> m_versions;
    std::vector<TeamMember> m_members;
    std::vector<ReviewComment> m_comments;
    std::vector<std::pair<std::string, int>> m_labels; // name, color
    std::unordered_map<std::string, std::vector<std::string>> m_itemLabels; // item path -> label names
    std::unordered_map<std::string, std::chrono::system_clock::time_point> m_syncTimes;
    SyncCallback m_syncCallback;

    std::string generateVersionId() const;
};

} // namespace FreeEffect
