#include "render_queue.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <powrprof.h>
#endif

namespace FreeEffect {

void RenderQueue::addItem(const RenderItem& item) {
    m_items.push_back(item);
}

void RenderQueue::removeItem(int index) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        m_items.erase(m_items.begin() + index);
    }
}

void RenderQueue::clear() {
    m_items.clear();
}

void RenderQueue::setItemOutputPath(int index, const std::string& path) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        m_items[index].outputPath = path;
    }
}

void RenderQueue::setItemFormat(int index, const std::string& format) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        m_items[index].format = format;
    }
}

void RenderQueue::setItemCodec(int index, const std::string& codec) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        m_items[index].codec = codec;
    }
}

void RenderQueue::setItemQuality(int index, int quality) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        m_items[index].quality = quality;
    }
}

void RenderQueue::startRender(int index, std::function<void(int, double)> progressCallback) {
    if (index < 0 || index >= static_cast<int>(m_items.size())) return;
    if (m_rendering) return;

    m_rendering = true;
    m_cancelled = false;

    auto& item = m_items[index];
    item.status = RenderStatus::Rendering;
    item.progress = 0.0;

    renderItem(item, progressCallback);

    if (m_cancelled) {
        item.status = RenderStatus::Cancelled;
    } else {
        item.status = RenderStatus::Complete;
        item.progress = 1.0;
    }

    m_rendering = false;

    if (!m_cancelled) {
        executePostRenderAction();
    }
}

void RenderQueue::startRenderWithProgress(int index,
    std::function<void(const std::string& status, double progress)> callback) {
    if (index < 0 || index >= static_cast<int>(m_items.size())) return;
    if (m_rendering) return;

    m_rendering = true;
    m_cancelled = false;

    auto& item = m_items[index];
    item.status = RenderStatus::Rendering;
    item.progress = 0.0;

    renderItemWithProgress(item, callback);

    if (m_cancelled) {
        item.status = RenderStatus::Cancelled;
        if (callback) callback("Cancelled", 0);
    } else {
        item.status = RenderStatus::Complete;
        item.progress = 1.0;
        if (callback) callback("Complete", 1.0);
    }

    m_rendering = false;

    if (!m_cancelled) {
        executePostRenderAction();
    }
}

void RenderQueue::cancelRender() {
    m_cancelled = true;
}

void RenderQueue::renderItem(RenderItem& item, std::function<void(int, double)> progress) {
    int startFrame = item.startFrame;
    int endFrame = item.endFrame;
    if (endFrame < 0) endFrame = startFrame + 100;

    // Adjust for render region
    if (item.renderRegion == RenderRegion::WorkArea) {
        // Work area bounds would be provided by composition
        // For now, use custom values
    } else if (item.renderRegion == RenderRegion::CustomRange) {
        startFrame = static_cast<int>(item.customRegionStart);
        endFrame = item.customRegionEnd >= 0 ? static_cast<int>(item.customRegionEnd) : endFrame;
    }

    int totalFrames = endFrame - startFrame;
    if (totalFrames <= 0) totalFrames = 1;

    if (item.multiFrameRendering && item.maxParallelFrames > 1) {
        // Multi-frame rendering: render frames in parallel batches
        int batchSize = std::min(item.maxParallelFrames, totalFrames);
        std::vector<std::thread> threads;
        std::vector<bool> frameDone(totalFrames, false);
        int nextFrame = 0;
        int completedFrames = 0;

        while (completedFrames < totalFrames) {
            if (m_cancelled) break;

            // Launch threads for batch
            threads.clear();
            for (int b = 0; b < batchSize && nextFrame < totalFrames; ++b, ++nextFrame) {
                int frameNum = startFrame + nextFrame;
                int frameIdx = nextFrame;
                threads.emplace_back([&, frameNum, frameIdx]() {
                    // Simulate rendering a single frame
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    frameDone[frameIdx] = true;
                });
            }

            // Wait for batch to complete
            for (auto& t : threads) {
                if (t.joinable()) t.join();
            }

            // Update progress
            completedFrames += batchSize;
            item.progress = static_cast<double>(completedFrames) / totalFrames;

            if (progress) {
                progress(startFrame + completedFrames, item.progress);
            }
        }
    } else {
        // Single-frame rendering
        for (int f = startFrame; f <= endFrame; ++f) {
            if (m_cancelled) return;

            // Skip existing files if enabled
            if (item.skipExistingFiles && !item.outputPath.empty()) {
                char framePath[1024];
                snprintf(framePath, sizeof(framePath), "%s_%05d",
                         item.outputPath.c_str(), f);
                if (fileExists(framePath)) {
                    item.progress = static_cast<double>(f - startFrame + 1) / totalFrames;
                    if (progress) progress(f, item.progress);
                    continue;
                }
            }

            item.progress = static_cast<double>(f - startFrame) / totalFrames;

            if (progress) {
                progress(f, item.progress);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void RenderQueue::renderItemWithProgress(RenderItem& item,
    std::function<void(const std::string&, double)> callback) {
    int startFrame = item.startFrame;
    int endFrame = item.endFrame;
    if (endFrame < 0) endFrame = startFrame + 100;

    // Adjust for render region
    if (item.renderRegion == RenderRegion::CustomRange) {
        startFrame = static_cast<int>(item.customRegionStart);
        endFrame = item.customRegionEnd >= 0 ? static_cast<int>(item.customRegionEnd) : endFrame;
    }

    int totalFrames = endFrame - startFrame;
    if (totalFrames <= 0) totalFrames = 1;

    for (int f = startFrame; f <= endFrame; ++f) {
        if (m_cancelled) return;

        if (item.skipExistingFiles && !item.outputPath.empty()) {
            char framePath[1024];
            snprintf(framePath, sizeof(framePath), "%s_%05d", item.outputPath.c_str(), f);
            if (fileExists(framePath)) {
                item.progress = static_cast<double>(f - startFrame + 1) / totalFrames;
                if (callback) {
                    std::string status = "Skipping frame " + std::to_string(f);
                    callback(status, item.progress);
                }
                continue;
            }
        }

        item.progress = static_cast<double>(f - startFrame) / totalFrames;

        if (callback) {
            std::string status = "Rendering frame " + std::to_string(f) + " / " + std::to_string(endFrame);
            callback(status, item.progress);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void RenderQueue::saveAsTemplate(const std::string& name) const {
    std::string path = name + ".render_template";
    std::ofstream file(path);
    if (!file.is_open()) return;

    file << "FreeEffect Render Template\n";
    file << "ItemCount: " << m_items.size() << "\n";
    file << "PostAction: " << static_cast<int>(m_postRenderAction) << "\n";
    file << "SkipExisting: " << (m_skipExisting ? "1" : "0") << "\n";
    file << "MaxParallelFrames: " << m_maxParallelFrames << "\n\n";

    for (size_t i = 0; i < m_items.size(); ++i) {
        const auto& item = m_items[i];
        file << "Item[" << i << "]\n";
        file << "  Name: " << item.name << "\n";
        file << "  OutputPath: " << item.outputPath << "\n";
        file << "  Format: " << item.format << "\n";
        file << "  Codec: " << item.codec << "\n";
        file << "  Quality: " << item.quality << "\n";
        file << "  StartFrame: " << item.startFrame << "\n";
        file << "  EndFrame: " << item.endFrame << "\n";
        file << "  StartTime: " << item.startTime << "\n";
        file << "  EndTime: " << item.endTime << "\n";
        file << "  CompId: " << item.compId << "\n";
        file << "  RenderRegion: " << static_cast<int>(item.renderRegion) << "\n";
        file << "  CustomRegionStart: " << item.customRegionStart << "\n";
        file << "  CustomRegionEnd: " << item.customRegionEnd << "\n";
        file << "  SkipExisting: " << (item.skipExistingFiles ? "1" : "0") << "\n";
        file << "  MultiFrameRendering: " << (item.multiFrameRendering ? "1" : "0") << "\n";
        file << "  MaxParallelFrames: " << item.maxParallelFrames << "\n";
        file << "  OutputChannels: " << static_cast<int>(item.outputModule.channels) << "\n";
        file << "  OutputColorDepth: " << item.outputModule.colorDepth << "\n";
        file << "  OutputIncludeAlpha: " << (item.outputModule.includeAlpha ? "1" : "0") << "\n";
        file << "  OutputAudioFormat: " << item.outputModule.audioFormat << "\n";
        file << "  OutputAudioBitrate: " << item.outputModule.audioBitrate << "\n";
        file << "\n";
    }
}

RenderQueue RenderQueue::loadTemplate(const std::string& name) {
    RenderQueue queue;
    std::string path = name + ".render_template";
    std::ifstream file(path);
    if (!file.is_open()) return queue;

    std::string line;
    RenderItem currentItem;
    bool inItem = false;

    while (std::getline(file, line)) {
        if (line.find("PostAction: ") == 0) {
            queue.m_postRenderAction = static_cast<PostRenderAction>(std::stoi(line.substr(12)));
        } else if (line.find("SkipExisting: ") == 0) {
            queue.m_skipExisting = (line.substr(14) == "1");
        } else if (line.find("MaxParallelFrames: ") == 0) {
            queue.m_maxParallelFrames = std::stoi(line.substr(19));
        } else if (line.find("Item[") == 0) {
            if (inItem) queue.addItem(currentItem);
            currentItem = RenderItem();
            inItem = true;
        } else if (inItem) {
            if (line.find("  Name: ") == 0) currentItem.name = line.substr(8);
            else if (line.find("  OutputPath: ") == 0) currentItem.outputPath = line.substr(14);
            else if (line.find("  Format: ") == 0) currentItem.format = line.substr(10);
            else if (line.find("  Codec: ") == 0) currentItem.codec = line.substr(9);
            else if (line.find("  Quality: ") == 0) currentItem.quality = std::stoi(line.substr(11));
            else if (line.find("  StartFrame: ") == 0) currentItem.startFrame = std::stoi(line.substr(14));
            else if (line.find("  EndFrame: ") == 0) currentItem.endFrame = std::stoi(line.substr(12));
            else if (line.find("  StartTime: ") == 0) currentItem.startTime = std::stod(line.substr(13));
            else if (line.find("  EndTime: ") == 0) currentItem.endTime = std::stod(line.substr(11));
            else if (line.find("  CompId: ") == 0) currentItem.compId = line.substr(10);
            else if (line.find("  RenderRegion: ") == 0) currentItem.renderRegion = static_cast<RenderRegion>(std::stoi(line.substr(16)));
            else if (line.find("  CustomRegionStart: ") == 0) currentItem.customRegionStart = std::stod(line.substr(21));
            else if (line.find("  CustomRegionEnd: ") == 0) currentItem.customRegionEnd = std::stod(line.substr(19));
            else if (line.find("  SkipExisting: ") == 0) currentItem.skipExistingFiles = (line.substr(16) == "1");
            else if (line.find("  MultiFrameRendering: ") == 0) currentItem.multiFrameRendering = (line.substr(22) == "1");
            else if (line.find("  MaxParallelFrames: ") == 0) currentItem.maxParallelFrames = std::stoi(line.substr(20));
            else if (line.find("  OutputChannels: ") == 0) currentItem.outputModule.channels = static_cast<OutputChannelMode>(std::stoi(line.substr(18)));
            else if (line.find("  OutputColorDepth: ") == 0) currentItem.outputModule.colorDepth = std::stoi(line.substr(20));
            else if (line.find("  OutputIncludeAlpha: ") == 0) currentItem.outputModule.includeAlpha = (line.substr(21) == "1");
            else if (line.find("  OutputAudioFormat: ") == 0) currentItem.outputModule.audioFormat = line.substr(20);
            else if (line.find("  OutputAudioBitrate: ") == 0) currentItem.outputModule.audioBitrate = std::stoi(line.substr(21));
        }
    }
    if (inItem) queue.addItem(currentItem);

    return queue;
}

void RenderQueue::saveRenderTemplate(const RenderTemplate& templ) {
    std::string path = templ.name + ".render_template";
    std::ofstream file(path);
    if (!file.is_open()) return;

    file << "FreeEffect Render Template v2\n";
    file << "TemplateName: " << templ.name << "\n";
    file << "DefaultRegion: " << static_cast<int>(templ.defaultRegion) << "\n";
    file << "PostAction: " << static_cast<int>(templ.postAction) << "\n";
    file << "Format: " << templ.outputSettings.format << "\n";
    file << "Codec: " << templ.outputSettings.codec << "\n";
    file << "Quality: " << templ.outputSettings.quality << "\n";
    file << "Channels: " << static_cast<int>(templ.outputSettings.channels) << "\n";
    file << "ColorDepth: " << templ.outputSettings.colorDepth << "\n";
    file << "IncludeAlpha: " << (templ.outputSettings.includeAlpha ? "1" : "0") << "\n";
    file << "AudioFormat: " << templ.outputSettings.audioFormat << "\n";
    file << "AudioBitrate: " << templ.outputSettings.audioBitrate << "\n";
    file << "AudioSampleRate: " << templ.outputSettings.audioSampleRate << "\n";
}

RenderTemplate RenderQueue::loadRenderTemplate(const std::string& name) {
    RenderTemplate templ;
    templ.name = name;
    std::string path = name + ".render_template";
    std::ifstream file(path);
    if (!file.is_open()) return templ;

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("TemplateName: ") == 0) templ.name = line.substr(14);
        else if (line.find("DefaultRegion: ") == 0) templ.defaultRegion = static_cast<RenderRegion>(std::stoi(line.substr(15)));
        else if (line.find("PostAction: ") == 0) templ.postAction = static_cast<PostRenderAction>(std::stoi(line.substr(12)));
        else if (line.find("Format: ") == 0) templ.outputSettings.format = line.substr(8);
        else if (line.find("Codec: ") == 0) templ.outputSettings.codec = line.substr(7);
        else if (line.find("Quality: ") == 0) templ.outputSettings.quality = std::stoi(line.substr(9));
        else if (line.find("Channels: ") == 0) templ.outputSettings.channels = static_cast<OutputChannelMode>(std::stoi(line.substr(10)));
        else if (line.find("ColorDepth: ") == 0) templ.outputSettings.colorDepth = std::stoi(line.substr(12));
        else if (line.find("IncludeAlpha: ") == 0) templ.outputSettings.includeAlpha = (line.substr(14) == "1");
        else if (line.find("AudioFormat: ") == 0) templ.outputSettings.audioFormat = line.substr(13);
        else if (line.find("AudioBitrate: ") == 0) templ.outputSettings.audioBitrate = std::stoi(line.substr(14));
        else if (line.find("AudioSampleRate: ") == 0) templ.outputSettings.audioSampleRate = std::stoi(line.substr(17));
    }

    return templ;
}

std::vector<std::string> RenderQueue::getTemplateNames() {
    std::vector<std::string> names;
    // Scan current directory for .render_template files
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        if (entry.is_regular_file()) {
            std::string fname = entry.path().filename().string();
            if (fname.size() > 18 && fname.substr(fname.size() - 18) == ".render_template") {
                names.push_back(fname.substr(0, fname.size() - 18));
            }
        }
    }
    return names;
}

void RenderQueue::executePostRenderAction() {
    switch (m_postRenderAction) {
        case PostRenderAction::None:
            break;

        case PostRenderAction::ShutDown:
#ifdef _WIN32
            {
                HANDLE hToken;
                TOKEN_PRIVILEGES tp;
                if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
                    LookupPrivilegeValue(nullptr, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid);
                    tp.PrivilegeCount = 1;
                    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                    AdjustTokenPrivileges(hToken, FALSE, &tp, 0, nullptr, nullptr);
                    InitiateSystemShutdownEx(nullptr, nullptr, 0, TRUE, FALSE, SHTDN_REASON_MAJOR_APPLICATION);
                }
            }
#elif defined(__APPLE__)
            system("osascript -e 'tell app \"System Events\" to shut down'");
#elif defined(__linux__)
            system("shutdown -h now");
#endif
            break;

        case PostRenderAction::Sleep:
#ifdef _WIN32
            SetSuspendState(FALSE, TRUE, FALSE);
#elif defined(__APPLE__)
            system("pmset sleepnow");
#elif defined(__linux__)
            system("systemctl suspend");
#endif
            break;

        case PostRenderAction::OpenInPlayer:
            if (!m_items.empty()) {
                std::string path = m_items.back().outputPath;
#ifdef _WIN32
                std::string cmd = "start \"\" \"" + path + "\"";
                system(cmd.c_str());
#elif defined(__APPLE__)
                std::string cmd = "open \"" + path + "\"";
                system(cmd.c_str());
#elif defined(__linux__)
                std::string cmd = "xdg-open \"" + path + "\"";
                system(cmd.c_str());
#endif
            }
            break;

        case PostRenderAction::PlaySound:
            // Play a notification sound
#ifdef _WIN32
            MessageBeep(MB_OK);
#elif defined(__APPLE__)
            system("afplay /System/Library/Sounds/Glass.aiff");
#elif defined(__linux__)
            system("paplay /usr/share/sounds/freedesktop/stereo/complete.oga 2>/dev/null || true");
#endif
            break;
    }
}

void RenderQueue::setItemRenderRegion(RenderItem& item, RenderRegion region,
                                       double customStart, double customEnd) {
    item.renderRegion = region;
    item.customRegionStart = customStart;
    item.customRegionEnd = customEnd;
}

void RenderQueue::setItemWorkArea(RenderItem& item, double workAreaStart, double workAreaEnd) {
    item.renderRegion = RenderRegion::WorkArea;
    item.customRegionStart = workAreaStart;
    item.customRegionEnd = workAreaEnd;
}

void RenderQueue::setOutputModule(RenderItem& item, const OutputModuleSettings& settings) {
    item.outputModule = settings;
    item.format = settings.format;
    item.codec = settings.codec;
    item.quality = settings.quality;
}

OutputModuleSettings RenderQueue::getOutputModule(const RenderItem& item) {
    return item.outputModule;
}

bool RenderQueue::fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

bool RenderQueue::validateRenderItem(const RenderItem& item, std::string& errorMessage) {
    if (item.outputPath.empty()) {
        errorMessage = "Output path is not set";
        return false;
    }

    // Check if output directory exists
    std::string dir = std::filesystem::path(item.outputPath).parent_path().string();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        errorMessage = "Output directory does not exist: " + dir;
        return false;
    }

    if (item.format.empty()) {
        errorMessage = "Output format is not set";
        return false;
    }

    if (item.renderRegion == RenderRegion::CustomRange) {
        if (item.customRegionEnd < 0 || item.customRegionEnd <= item.customRegionStart) {
            errorMessage = "Invalid custom render range";
            return false;
        }
    }

    return true;
}

int RenderQueue::calculateActualStartFrame(const RenderItem& item, double compFrameRate) const {
    int start = item.startFrame;
    if (item.renderRegion == RenderRegion::WorkArea || item.renderRegion == RenderRegion::CustomRange) {
        start = static_cast<int>(item.customRegionStart * compFrameRate);
    }
    return start;
}

int RenderQueue::calculateActualEndFrame(const RenderItem& item, double compFrameRate) const {
    int end = item.endFrame;
    if (end < 0) return item.startFrame + 100;
    if (item.renderRegion == RenderRegion::WorkArea || item.renderRegion == RenderRegion::CustomRange) {
        if (item.customRegionEnd >= 0) {
            end = static_cast<int>(item.customRegionEnd * compFrameRate);
        }
    }
    return end;
}

} // namespace FreeEffect
