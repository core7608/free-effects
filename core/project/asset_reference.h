#pragma once

#include "../timeline/types.h"
#include <string>

namespace FreeEffect {

enum class AssetType {
    Video,
    Image,
    Audio,
    Composition
};

class AssetReference {
public:
    AssetReference(const std::string& path, const std::string& name, AssetType type);
    
    const UUID& getId() const { return m_id; }
    
    const std::string& getPath() const { return m_path; }
    void setPath(const std::string& path) { m_path = path; }
    
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    
    AssetType getType() const { return m_type; }
    
    AssetStatus getStatus() const { return m_status; }
    void setStatus(AssetStatus status) { m_status = status; }
    
    double getDuration() const { return m_duration; }
    void setDuration(double duration) { m_duration = duration; }
    
    int getWidth() const { return m_width; }
    void setWidth(int width) { m_width = width; }
    
    int getHeight() const { return m_height; }
    void setHeight(int height) { m_height = height; }
    
    double getFrameRate() const { return m_frameRate; }
    void setFrameRate(double fps) { m_frameRate = fps; }

private:
    UUID m_id;
    std::string m_path;
    std::string m_name;
    AssetType m_type;
    AssetStatus m_status = AssetStatus::Available;
    double m_duration = 0.0;
    int m_width = 0;
    int m_height = 0;
    double m_frameRate = 0.0;
};

using AssetPtr = std::shared_ptr<AssetReference>;

} // namespace FreeEffect
