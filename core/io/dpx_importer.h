#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace FreeEffect {

enum class DPXPixelStorage {
    Pack,
    FilledA,
    FilledB
};

enum class DPXOrientation {
    LeftToRightTopToBottom,
    RightToLeftTopToBottom,
    LeftToRightBottomToTop,
    RightToLeftBottomToTop
};

enum class DPXBitDepth {
    Bits1,
    Bits8,
    Bits10,
    Bits12,
    Bits16,
    Bits32Float
};

enum class DPXPacking {
    PackWord1,
    PackWord32,
    PackWord32Filled
};

struct DPXImageHeader {
    int width = 0;
    int height = 0;
    int bitsPerElement = 10;
    int elementsPerPixel = 3;
    DPXOrientation orientation = DPXOrientation::LeftToRightTopToBottom;
    DPXPixelStorage storage = DPXPixelStorage::Pack;
    DPXBitDepth bitDepth = DPXBitDepth::Bits10;
    DPXPacking packing = DPXPacking::PackWord1;
    int linePacking = 0;
    uint32_t descriptor = 50;
    uint32_t transfer = 2;
    uint32_t colorimetric = 1;
    uint32_t dataSign = 0;
    float lowQuantity = 0.0f;
    float highQuantity = 1.0f;
    std::string filename;
    std::string date;
    std::string creator;
    std::string project;
    std::string copyright;
    bool isLittleEndian = false;
    uint32_t dwordPadding = 0;
};

struct DPXImageData {
    std::vector<float> rgbPixels;
    int width = 0;
    int height = 0;
    int channels = 3;
    int bitsPerComponent = 10;
};

class DPXImporter {
public:
    DPXImporter() = default;
    ~DPXImporter() = default;

    bool open(const std::string& filePath);
    bool readHeader();
    bool readPixels(DPXImageData& imageData);
    void close();

    const DPXImageHeader& getHeader() const { return m_header; }
    bool isOpen() const { return m_fileOpen; }
    std::string getErrorMessage() const { return m_errorMessage; }

    static bool isDPXFile(const std::string& filePath);

private:
    uint16_t readU16(const uint8_t* p) const;
    uint32_t readU32(const uint8_t* p) const;
    uint32_t industryHeaderLength() const;
    uint32_t fileHeaderLength() const;

    std::string m_filePath;
    FILE* m_file = nullptr;
    bool m_fileOpen = false;
    bool m_headerRead = false;
    bool m_littleEndian = true;
    DPXImageHeader m_header;
    std::string m_errorMessage;
    std::vector<uint8_t> m_rawData;
};

} // namespace FreeEffect
