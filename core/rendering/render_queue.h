#pragma once
#include "../timeline/composition.h"
#include <string>
#include <vector>
#include <functional>

namespace FreeEffect {

enum class RenderStatus { Queued, Rendering, Complete, Failed, Cancelled };

enum class RenderRegion { WorkArea, Entire, CustomRange };

enum class PostRenderAction { None, ShutDown, Sleep, OpenInPlayer, PlaySound };

enum class OutputChannelMode { RGB, RGBA, Alpha, Red, Green, Blue };

struct OutputModuleSettings {
    std::string format = "mp4";
    std::string codec = "h264";
    int quality = 100;
    OutputChannelMode channels = OutputChannelMode::RGB;
    int colorDepth = 8; // 8, 16, 32
    bool includeAlpha = false;
    std::string audioFormat = "aac";
    int audioBitrate = 192;
    int audioSampleRate = 48000;
};

struct RenderItem {
    std::string name;
    std::string outputPath;
    std::string format = "mp4";
    std::string codec = "h264";
    int quality = 100;
    int startFrame = 0;
    int endFrame = -1;
    double startTime = 0;
    double endTime = -1;
    RenderStatus status = RenderStatus::Queued;
    double progress = 0.0;
    UUID compId;

    RenderRegion renderRegion = RenderRegion::Entire;
    double customRegionStart = 0.0;
    double customRegionEnd = -1.0;
    bool skipExistingFiles = false;
    bool multiFrameRendering = false;
    int maxParallelFrames = 4;

    OutputModuleSettings outputModule;
};

struct RenderTemplate {
    std::string name;
    OutputModuleSettings outputSettings;
    RenderRegion defaultRegion = RenderRegion::Entire;
    PostRenderAction postAction = PostRenderAction::None;
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

    // Render templates
    void saveRenderTemplate(const RenderTemplate& templ);
    static RenderTemplate loadRenderTemplate(const std::string& name);
    static std::vector<std::string> getTemplateNames();

    // Post-render actions
    void setPostRenderAction(PostRenderAction action) { m_postRenderAction = action; }
    PostRenderAction getPostRenderAction() const { return m_postRenderAction; }
    void executePostRenderAction();

    // Skip existing files
    void setSkipExisting(bool skip) { m_skipExisting = skip; }
    bool getSkipExisting() const { return m_skipExisting; }

    // Render region helpers
    static void setItemRenderRegion(RenderItem& item, RenderRegion region,
                                     double customStart = 0, double customEnd = -1);
    static void setItemWorkArea(RenderItem& item, double workAreaStart, double workAreaEnd);

    // Output module settings
    static void setOutputModule(RenderItem& item, const OutputModuleSettings& settings);
    static OutputModuleSettings getOutputModule(const RenderItem& item);

    // Multi-frame rendering
    void setMaxParallelFrames(int max) { m_maxParallelFrames = max; }
    int getMaxParallelFrames() const { return m_maxParallelFrames; }

    // Render with progress callback (enhanced)
    void startRenderWithProgress(int index,
                                  std::function<void(const std::string& status, double progress)> callback);

    // Check if output file already exists
    static bool fileExists(const std::string& path);

    // Validate render item
    static bool validateRenderItem(const RenderItem& item, std::string& errorMessage);

private:
    std::vector<RenderItem> m_items;
    bool m_rendering = false;
    bool m_cancelled = false;
    PostRenderAction m_postRenderAction = PostRenderAction::None;
    bool m_skipExisting = false;
    int m_maxParallelFrames = 4;

    void renderItem(RenderItem& item, std::function<void(int, double)> progress);
    void renderItemWithProgress(RenderItem& item,
                                std::function<void(const std::string&, double)> callback);
    int calculateActualStartFrame(const RenderItem& item, double compFrameRate) const;
    int calculateActualEndFrame(const RenderItem& item, double compFrameRate) const;
};

} // namespace FreeEffect
