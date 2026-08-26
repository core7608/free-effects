#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace FreeEffect {

struct PDFImageInfo {
    int width = 0;
    int height = 0;
    int bitsPerComponent = 8;
    int colorSpace = 3;
    std::vector<uint8_t> pixelData;
};

struct PDFPageInfo {
    int pageNumber = 0;
    int width = 0;
    int height = 0;
    int imageCount = 0;
    std::string title;
};

class PDFImporter {
public:
    PDFImporter() = default;
    ~PDFImporter() = default;

    bool open(const std::string& filePath);
    bool readHeader();
    int getPageCount() const { return static_cast<int>(m_pages.size()); }
    bool extractPageImage(int pageNumber, PDFImageInfo& image);
    bool extractAllImages(std::vector<PDFImageInfo>& images);
    void close();

    const std::vector<PDFPageInfo>& getPages() const { return m_pages; }
    bool isOpen() const { return m_fileOpen; }
    std::string getErrorMessage() const { return m_errorMessage; }

    static bool isPDFFile(const std::string& filePath);

private:
    bool parsePDFStructure();
    bool findXRef();
    bool parseXRef();
    bool parsePageObjects();
    bool decompressStream(const std::string& streamData, std::vector<uint8_t>& output);

    std::string m_filePath;
    bool m_fileOpen = false;
    std::string m_errorMessage;
    std::vector<uint8_t> m_rawData;
    std::vector<PDFPageInfo> m_pages;

    struct XRefEntry {
        uint32_t byteOffset = 0;
        uint16_t generation = 0;
        bool inUse = true;
    };
    std::vector<XRefEntry> m_xref;
    int64_t m_xrefOffset = 0;
};

} // namespace FreeEffect
