#pragma once
#include "../timeline/composition.h"
#include <string>
#include <vector>
#include <functional>

namespace FreeEffect {

enum class RenderStatus { Queued, Rendering, Complete, Failed, Cancelled };

struct RenderItem {
    std::string name;
    std::string outputPath;
    std::string format = "mp4";       // mp4, mov, avi, png sequence, exr sequence, prores
    std::string codec = "h264";       // h264, h265, prores422, prores4444, dnxhd
    int quality = 100;
    int startFrame = 0;
    int endFrame = -1;                // -1 = comp end
    double startTime = 0;
    double endTime = -1;
    RenderStatus status = RenderStatus::Queued;
    double progress = 0.0;
    UUID compId;
};

class RenderQueue {
public:
    void addItem(const RenderItem& item);
    void removeItem(int index);
    void clear();
    
    const std::vector<RenderItem>& getItems() const { return m_items; }
    std::vector<RenderItem>& getItems() { return m_items; }
    
    void setItemOutputPath(int index, const std::string& path);
    void setItemFormat(int index, const std::string& format);
    void setItemCodec(int index, const std::string& codec);
    void setItemQuality(int index, int quality);
    
    void startRender(int index, std::function<void(int, double)> progressCallback = nullptr);
    void cancelRender();
    bool isRendering() const { return m_rendering; }
    
    // Template support
    void saveAsTemplate(const std::string& name) const;
    static RenderQueue loadTemplate(const std::string& name);

private:
    std::vector<RenderItem> m_items;
    bool m_rendering = false;
    bool m_cancelled = false;
    
    void renderItem(RenderItem& item, std::function<void(int, double)> progress);
};

} // namespace FreeEffect
