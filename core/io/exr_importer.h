#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace FreeEffect {

enum class EXRPixelType {
    Half,
    Float,
    UInt
};

enum class EXRCompression {
    None,
    Zip,
    Zips,
    Piz,
    Rle,
    B44,
    B44A,
    DWAA,
    DWAB
};

struct EXRChannelInfo {
    std::string name;
    EXRPixelType pixelType = EXRPixelType::Half;
    int xSampling = 1;
    int ySampling = 1;
};

struct EXRImageHeader {
    int width = 0;
    int height = 0;
    int dataWindowXMin = 0;
    int dataWindowYMin = 0;
    int dataWindowXMax = 0;
    int dataWindowYMax = 0;
    int displayWindowXMin = 0;
    int displayWindowYMin = 0;
    int displayWindowXMax = 0;
    int displayWindowYMax = 0;
    float pixelAspectRatio = 1.0f;
    float screenWindowWidth = 1.0f;
    EXRCompression compression = EXRCompression::Zip;
    bool isMultiPart = false;
    bool isDeep = false;
    std::vector<EXRChannelInfo> channels;
    std::vector<std::pair<std::string, std::string>> attributes;
};

struct EXRImageData {
    std::vector<float> rgbaPixels;
    std::vector<float> depthPixels;
    std::vector<float> normalPixels;
    int width = 0;
    int height = 0;
    int channels = 4;
    bool hasDepth = false;
    bool hasNormals = false;
};

class EXRImporter {
public:
    EXRImporter() = default;
    ~EXRImporter() = default;

    bool open(const std::string& filePath);
    bool readHeader();
    bool readPixels(EXRImageData& imageData);
    void close();

    const EXRImageHeader& getHeader() const { return m_header; }
    bool isOpen() const { return m_fileOpen; }
    std::string getErrorMessage() const { return m_errorMessage; }

    static bool isEXRFile(const std::string& filePath);

private:
    bool readMagicNumber();
    bool readVersion();
    bool readAttributes();
    bool readChannelList();
    bool readCompressionAttribute(const std::string& data, size_t& offset);
    bool readBox2iAttribute(const std::string& data, size_t& offset, int& xMin, int& yMin, int& xMax, int& yMax);
    bool readFloatAttribute(const std::string& data, size_t& offset, float& value);
    bool readStringAttribute(const std::string& data, size_t& offset, std::string& value);

    float halfToFloat(uint16_t halfVal);
    uint16_t floatToHalf(float val);

    bool decompressZip(std::vector<uint8_t>& output, const uint8_t* input, size_t inputSize, size_t uncompressedSize);
    bool decompressRle(std::vector<uint8_t>& output, const uint8_t* input, size_t inputSize, size_t uncompressedSize);
    bool decompressPiz(std::vector<uint8_t>& output, const uint8_t* input, size_t inputSize, size_t uncompressedSize);

    std::string m_filePath;
    FILE* m_file = nullptr;
    bool m_fileOpen = false;
    bool m_littleEndian = true;
    bool m_headerRead = false;
    EXRImageHeader m_header;
    std::string m_errorMessage;
    std::vector<uint8_t> m_rawData;
};

} // namespace FreeEffect
