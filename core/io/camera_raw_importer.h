#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace FreeEffect {

enum class RawFormat {
    Unknown,
    CR2,    // Canon
    NEF,    // Nikon
    ARW,    // Sony
    DNG,    // Adobe
    ORF,    // Olympus
    RW2,    // Panasonic
    RAF,    // Fuji
    PEF     // Pentax
};

struct RawMetadata {
    RawFormat format = RawFormat::Unknown;
    std::string make;
    std::string model;
    int width = 0;
    int height = 0;
    int bitsPerComponent = 14;
    double iso = 0;
    double shutterSpeed = 0;
    double aperture = 0;
    double focalLength = 0;
    std::string dateTime;
    std::string colorSpace;
    bool hasThumbnail = false;
    int thumbnailWidth = 0;
    int thumbnailHeight = 0;
    std::vector<uint8_t> thumbnailData;
};

class CameraRawImporter {
public:
    CameraRawImporter() = default;
    ~CameraRawImporter() = default;

    bool open(const std::string& filePath);
    bool readMetadata();
    bool extractThumbnail(std::vector<uint8_t>& thumbnailData, int& width, int& height);
    void close();

    const RawMetadata& getMetadata() const { return m_metadata; }
    bool isOpen() const { return m_fileOpen; }
    std::string getErrorMessage() const { return m_errorMessage; }

    static bool isRAWFile(const std::string& filePath);
    static RawFormat detectFormat(const std::string& filePath);

private:
    bool parseCR2Header();
    bool parseNEFHeader();
    bool parseARWHeader();
    bool parseDNGHeader();
    bool parseTIFFHeader();

    std::string m_filePath;
    bool m_fileOpen = false;
    RawMetadata m_metadata;
    std::string m_errorMessage;
    std::vector<uint8_t> m_rawData;
    bool m_littleEndian = true;
};

} // namespace FreeEffect
