#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace FreeEffect {

struct HDRImageHeader {
    int width = 0;
    int height = 0;
    float exposure = 1.0f;
    bool isRadianceFormat = true;
    std::string format;
    std::string comment;
};

struct HDRImageData {
    std::vector<float> rgbPixels;
    int width = 0;
    int height = 0;
    int channels = 3;
    float exposure = 1.0f;
};

class HDRImporter {
public:
    HDRImporter() = default;
    ~HDRImporter() = default;

    bool open(const std::string& filePath);
    bool readHeader();
    bool readPixels(HDRImageData& imageData);
    void close();

    const HDRImageHeader& getHeader() const { return m_header; }
    bool isOpen() const { return m_fileOpen; }
    std::string getErrorMessage() const { return m_errorMessage; }

    static bool isHDRFile(const std::string& filePath);

private:
    bool parseHeader();
    bool decodeRGBE(const uint8_t* rgbe, float* rgb);

    std::string m_filePath;
    FILE* m_file = nullptr;
    bool m_fileOpen = false;
    bool m_headerRead = false;
    HDRImageHeader m_header;
    std::string m_errorMessage;
    std::vector<uint8_t> m_rawData;
    size_t m_pixelDataOffset = 0;
};

} // namespace FreeEffect
