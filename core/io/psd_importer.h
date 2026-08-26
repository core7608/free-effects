#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace FreeEffect {

struct PSDLayerInfo {
    std::string name;
    int width = 0, height = 0;
    int x = 0, y = 0;
    bool visible = true;
    double opacity = 100.0;
    int blendMode = 0;
    bool isAdjustmentLayer = false;
    bool isTextLayer = false;
    bool isShapeLayer = false;
    std::vector<uint8_t> pixelData;
};

struct PSDFileInfo {
    int width = 0, height = 0;
    int channels = 0;
    int bitDepth = 8;
    int colorMode = 0;
    std::vector<PSDLayerInfo> layers;
};

class PSDImporter {
public:
    PSDFileInfo parseHeader(const std::string& path);
    bool importLayers(const std::string& path, PSDFileInfo& info);
    std::vector<std::string> getLayerNames(const std::string& path);

private:
    bool readPSDHeader(FILE* f, PSDFileInfo& info);
    bool readPSDLayers(FILE* f, PSDFileInfo& info);
    bool decompressData(const std::vector<uint8_t>& compressed,
                        std::vector<uint8_t>& decompressed, int expectedSize);

    static uint16_t readU16BE(FILE* f);
    static uint32_t readU32BE(FILE* f);
    static void skipBytes(FILE* f, long n);
};

} // namespace FreeEffect
