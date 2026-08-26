#include "render_queue.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

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
}

void RenderQueue::cancelRender() {
    m_cancelled = true;
}

void RenderQueue::renderItem(RenderItem& item, std::function<void(int, double)> progress) {
    // Stub: simulate rendering frames
    int startFrame = item.startFrame;
    int endFrame = item.endFrame;
    if (endFrame < 0) endFrame = startFrame + 100;

    int totalFrames = endFrame - startFrame;
    if (totalFrames <= 0) totalFrames = 1;

    for (int f = startFrame; f <= endFrame; ++f) {
        if (m_cancelled) return;

        item.progress = static_cast<double>(f - startFrame) / totalFrames;

        if (progress) {
            progress(f, item.progress);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void RenderQueue::saveAsTemplate(const std::string& name) const {
    std::string path = name + ".render_template";
    std::ofstream file(path);
    if (!file.is_open()) return;

    file << "FreeEffect Render Template\n";
    file << "ItemCount: " << m_items.size() << "\n\n";

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
        file << "  CompId: " << item.compId << "\n\n";
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
        if (line.find("Item[") == 0) {
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
        }
    }
    if (inItem) queue.addItem(currentItem);

    return queue;
}

} // namespace FreeEffect
