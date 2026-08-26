#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace FreeEffect {

struct AILayerInfo {
    std::string name;
    int width = 0, height = 0;
    bool visible = true;
    std::vector<uint8_t> pixelData;
};

struct AIFileInfo {
    int width = 0, height = 0;
    std::string title;
    std::string creator;
    std::vector<AILayerInfo> layers;
    bool isPDF = false;
};

class AIImporter {
public:
    AIFileInfo parseFile(const std::string& path);
    bool importLayers(const std::string& path, AIFileInfo& info);

private:
    bool parseEPS(FILE* f, AIFileInfo& info);
    bool parsePDFHeader(FILE* f, AIFileInfo& info);
    bool parsePDFBody(FILE* f, AIFileInfo& info);
    static std::string readLine(FILE* f);
    static void trimInPlace(std::string& s);
};

} // namespace FreeEffect
