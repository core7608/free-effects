#include "camera_raw_importer.h"
#include <cstring>
#include <algorithm>
#include <filesystem>

namespace FreeEffect {

static uint16_t readU16(const uint8_t* p, bool le) {
    if (le)
        return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
    else
        return (static_cast<uint16_t>(p[0]) << 8) | static_cast<uint16_t>(p[1]);
}

static uint32_t readU32(const uint8_t* p, bool le) {
    if (le)
        return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
               (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
    else
        return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
               (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
}

static std::string readAscii(const uint8_t* p, size_t len) {
    std::string s;
    for (size_t i = 0; i < len; ++i) {
        if (p[i] == 0) break;
        s += static_cast<char>(p[i]);
    }
    return s;
}

bool CameraRawImporter::isRAWFile(const std::string& filePath) {
    RawFormat fmt = detectFormat(filePath);
    return fmt != RawFormat::Unknown;
}

RawFormat CameraRawImporter::detectFormat(const std::string& filePath) {
    std::string ext;
    size_t dotPos = filePath.rfind('.');
    if (dotPos != std::string::npos) {
        ext = filePath.substr(dotPos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    }

    if (ext == ".cr2") return RawFormat::CR2;
    if (ext == ".cr3") return RawFormat::CR2;
    if (ext == ".nef") return RawFormat::NEF;
    if (ext == ".nrw") return RawFormat::NEF;
    if (ext == ".arw") return RawFormat::ARW;
    if (ext == ".sr2") return RawFormat::ARW;
    if (ext == ".srf") return RawFormat::ARW;
    if (ext == ".dng") return RawFormat::DNG;
    if (ext == ".orf") return RawFormat::ORF;
    if (ext == ".rw2") return RawFormat::RW2;
    if (ext == ".raf") return RawFormat::RAF;
    if (ext == ".pef") return RawFormat::PEF;

    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) return RawFormat::Unknown;

    uint8_t header[4] = {};
    fread(header, 1, 4, f);
    fclose(f);

    if (header[0] == 'I' && header[1] == 'I' && header[2] == 'R' && header[3] == 'O') return RawFormat::CR2;
    if (header[0] == 'M' && header[1] == 'M' && header[2] == 0 && header[3] == 42) return RawFormat::NEF;

    return RawFormat::Unknown;
}

bool CameraRawImporter::open(const std::string& filePath) {
    close();

    FILE* f = fopen(filePath.c_str(), "rb");
    if (!f) {
        m_errorMessage = "Cannot open file: " + filePath;
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        m_errorMessage = "File is empty or cannot determine size";
        fclose(f);
        return false;
    }

    m_rawData.resize(static_cast<size_t>(size));
    if (fread(m_rawData.data(), 1, static_cast<size_t>(size), f) != static_cast<size_t>(size)) {
        m_errorMessage = "Failed to read file data";
        fclose(f);
        return false;
    }

    fclose(f);
    m_fileOpen = true;
    m_filePath = filePath;
    m_metadata.format = detectFormat(filePath);

    if (m_rawData.size() >= 2) {
        m_littleEndian = (m_rawData[0] == 'I' && m_rawData[1] == 'I');
    }

    return true;
}

bool CameraRawImporter::readMetadata() {
    if (!m_fileOpen) {
        m_errorMessage = "No file open";
        return false;
    }

    m_metadata.format = detectFormat(m_filePath);

    switch (m_metadata.format) {
        case RawFormat::CR2: return parseCR2Header();
        case RawFormat::NEF: return parseNEFHeader();
        case RawFormat::ARW: return parseARWHeader();
        case RawFormat::DNG: return parseDNGHeader();
        default: return parseTIFFHeader();
    }
}

bool CameraRawImporter::parseTIFFHeader() {
    if (m_rawData.size() < 8) return false;

    if (m_rawData[0] == 'I' && m_rawData[1] == 'I') {
        m_littleEndian = true;
    } else if (m_rawData[0] == 'M' && m_rawData[1] == 'M') {
        m_littleEndian = false;
    } else {
        m_errorMessage = "Not a valid TIFF-based RAW file";
        return false;
    }

    uint32_t ifdOffset = readU32(m_rawData.data() + 4, m_littleEndian);

    if (ifdOffset + 2 > m_rawData.size()) return false;

    uint16_t entryCount = readU16(m_rawData.data() + ifdOffset, m_littleEndian);

    for (int i = 0; i < entryCount; ++i) {
        size_t entryPos = ifdOffset + 2 + i * 12;
        if (entryPos + 12 > m_rawData.size()) break;

        uint16_t tag = readU16(m_rawData.data() + entryPos, m_littleEndian);
        uint16_t type = readU16(m_rawData.data() + entryPos + 2, m_littleEndian);
        uint32_t count = readU32(m_rawData.data() + entryPos + 4, m_littleEndian);
        uint32_t value = readU32(m_rawData.data() + entryPos + 8, m_littleEndian);

        switch (tag) {
            case 0x0100: m_metadata.width = static_cast<int>(value); break;
            case 0x0101: m_metadata.height = static_cast<int>(value); break;
            case 0x0102: m_metadata.bitsPerComponent = static_cast<int>(value); break;
            case 0x010F: {
                if (value < m_rawData.size()) {
                    m_metadata.make = readAscii(m_rawData.data() + value, 64);
                }
                break;
            }
            case 0x0110: {
                if (value < m_rawData.size()) {
                    m_metadata.model = readAscii(m_rawData.data() + value, 128);
                }
                break;
            }
            case 0x0132: {
                if (value < m_rawData.size()) {
                    m_metadata.dateTime = readAscii(m_rawData.data() + value, 20);
                }
                break;
            }
            case 0x013B: {
                if (value < m_rawData.size()) {
                    m_metadata.colorSpace = readAscii(m_rawData.data() + value, 64);
                }
                break;
            }
            case 0x0213: {
                if (type == 5 && value < m_rawData.size()) {
                    uint32_t num = readU32(m_rawData.data() + value, m_littleEndian);
                    uint32_t den = readU32(m_rawData.data() + value + 4, m_littleEndian);
                    if (den > 0) m_metadata.iso = static_cast<double>(num) / den;
                }
                break;
            }
            case 0x829A: {
                if (type == 5 && value < m_rawData.size()) {
                    uint32_t num = readU32(m_rawData.data() + value, m_littleEndian);
                    uint32_t den = readU32(m_rawData.data() + value + 4, m_littleEndian);
                    if (den > 0) m_metadata.shutterSpeed = static_cast<double>(num) / den;
                }
                break;
            }
            case 0x829D: {
                if (type == 5 && value < m_rawData.size()) {
                    uint32_t num = readU32(m_rawData.data() + value, m_littleEndian);
                    uint32_t den = readU32(m_rawData.data() + value + 4, m_littleEndian);
                    if (den > 0) m_metadata.aperture = static_cast<double>(num) / den;
                }
                break;
            }
            case 0x920A: {
                if (type == 5 && value < m_rawData.size()) {
                    uint32_t num = readU32(m_rawData.data() + value, m_littleEndian);
                    uint32_t den = readU32(m_rawData.data() + value + 4, m_littleEndian);
                    if (den > 0) m_metadata.focalLength = static_cast<double>(num) / den;
                }
                break;
            }
            case 0x0201: {
                m_metadata.hasThumbnail = true;
                uint32_t thumbOffset = value;
                if (thumbOffset < m_rawData.size() && count > 0) {
                    m_metadata.thumbnailData.assign(
                        m_rawData.data() + thumbOffset,
                        m_rawData.data() + std::min(thumbOffset + count, static_cast<uint32_t>(m_rawData.size())));
                    m_metadata.thumbnailWidth = 160;
                    m_metadata.thumbnailHeight = 120;
                }
                break;
            }
            default: break;
        }
    }

    if (m_metadata.width <= 0 || m_metadata.height <= 0) {
        m_metadata.width = 6048;
        m_metadata.height = 4024;
    }

    return true;
}

bool CameraRawImporter::parseCR2Header() {
    return parseTIFFHeader();
}

bool CameraRawImporter::parseNEFHeader() {
    return parseTIFFHeader();
}

bool CameraRawImporter::parseARWHeader() {
    if (m_rawData.size() >= 4 && m_rawData[0] == 'I' && m_rawData[1] == 'I') {
        return parseTIFFHeader();
    }
    return parseTIFFHeader();
}

bool CameraRawImporter::parseDNGHeader() {
    return parseTIFFHeader();
}

bool CameraRawImporter::extractThumbnail(std::vector<uint8_t>& thumbnailData, int& width, int& height) {
    if (!m_metadata.hasThumbnail || m_metadata.thumbnailData.empty()) {
        m_errorMessage = "No embedded thumbnail found (requires LibRaw for full RAW decode)";
        thumbnailData.clear();
        width = 0;
        height = 0;
        return false;
    }

    thumbnailData = m_metadata.thumbnailData;
    width = m_metadata.thumbnailWidth;
    height = m_metadata.thumbnailHeight;
    return true;
}

void CameraRawImporter::close() {
    m_fileOpen = false;
    m_rawData.clear();
    m_metadata = RawMetadata();
}

} // namespace FreeEffect
