#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace FreeEffect {

enum class TGAImageType : uint8_t {
    NoImage = 0,
    Indexed = 1,
    TrueColor = 2,
    Grayscale = 3,
    RLEIndexed = 9,
    RLETrueColor = 11,
    RLEGrayscale = 11
};

struct TGAHeader {
    uint8_t idLength = 0;
    uint8_t colorMapType = 0;
    uint8_t imageType = 0;
    uint16_t colorMapOrigin = 0;
    uint16_t colorMapLength = 0;
    uint8_t colorMapEntrySize = 0;
    uint16_t xOrigin = 0;
    uint16_t yOrigin = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t bitsPerPixel = 0;
    uint8_t imageDescriptor = 0;
    bool hasAlpha = false;
    bool isRLE = false;
    bool topToBottom = true;
    bool leftToRight = true;
    std::string imageId;
};

struct TGAImageData {
    std::vector<uint8_t> pixels;
    int width = 0;
    int height = 0;
    int bytesPerPixel = 4;
};

class TGAImporter {
public:
    TGAImporter() = default;
    ~TGAImporter() = default;

    bool open(const std::string& filePath);
    bool readHeader();
    bool readPixels(TGAImageData& imageData);
    void close();

    const TGAHeader& getHeader() const { return m_header; }
    bool isOpen() const { return m_fileOpen; }
    std::string getErrorMessage() const { return m_errorMessage; }

    static bool isTGAFile(const std::string& filePath);

private:
    std::string m_filePath;
    FILE* m_file = nullptr;
    bool m_fileOpen = false;
    bool m_headerRead = false;
    TGAHeader m_header;
    std::string m_errorMessage;
    std::vector<uint8_t> m_rawData;
};

} // namespace FreeEffect
