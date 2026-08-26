#include "ai_importer.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>

namespace FreeEffect {

AIFileInfo AIImporter::parseFile(const std::string& path) {
    AIFileInfo info;
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return info;

    // Check file magic
    char header[16] = {};
    size_t bytesRead = fread(header, 1, 15, f);
    header[bytesRead] = '\0';
    fseek(f, 0, SEEK_SET);

    // Check if it's a PDF-based AI file (%PDF)
    if (memcmp(header, "%PDF", 4) == 0) {
        info.isPDF = true;
        parsePDFHeader(f, info);
        fclose(f);
        return info;
    }

    // EPS-based AI file (%!PS-Adobe)
    if (memcmp(header, "%!", 2) == 0) {
        info.isPDF = false;
        parseEPS(f, info);
        fclose(f);
        return info;
    }

    fclose(f);
    return info;
}

bool AIImporter::parseEPS(FILE* f, AIFileInfo& info) {
    fseek(f, 0, SEEK_SET);
    std::string line;
    bool foundBBox = false;
    bool inData = false;

    // Read line by line looking for EPS/AI directives
    while (!feof(f)) {
        line = readLine(f);
        trimInPlace(line);

        // Title
        if (line.find("%%Title:") == 0) {
            info.title = line.substr(8);
            trimInPlace(info.title);
        }

        // Creator
        if (line.find("%%Creator:") == 0) {
            info.creator = line.substr(10);
            trimInPlace(info.creator);
        }

        // BoundingBox: llx lly urx ury
        if (line.find("%%BoundingBox:") == 0) {
            int llx = 0, lly = 0, urx = 0, ury = 0;
            if (sscanf(line.c_str(), "%%BoundingBox: %d %d %d %d", &llx, &lly, &urx, &ury) == 4) {
                info.width = urx - llx;
                info.height = ury - lly;
                foundBBox = true;
            }
        }

        // %%AI5_FileFormat (Adobe Illustrator file format version)
        if (line.find("%%AI5_FileFormat:") == 0) {
            // This confirms it's an AI file
        }

        // Look for raster image data (embedded images)
        if (line.find("%%BeginBinary:") == 0 || line.find("U1") == 0 ||
            line.find("%%BeginData:") == 0) {
            // Binary data section - skip for now
            while (!feof(f)) {
                line = readLine(f);
                if (line.find("%%EndBinary") != std::string::npos ||
                    line.find("%%EndData") != std::string::npos) {
                    break;
                }
            }
        }

        // Look for layer markers (AI uses groups for layers)
        if (line.find("/Layer") == 0 || line.find("/layer") == 0) {
            AILayerInfo layer;
            // Try to extract layer name from /Layer <name>
            size_t nameStart = line.find('(', line.find('/'));
            size_t nameEnd = line.find(')', nameStart);
            if (nameStart != std::string::npos && nameEnd != std::string::npos) {
                layer.name = line.substr(nameStart + 1, nameEnd - nameStart - 1);
            }
            layer.width = info.width;
            layer.height = info.height;
            info.layers.push_back(layer);
        }

        // Page device info
        if (line.find("%%BeginPageSetup") != std::string::npos) {
            inData = true;
        }
        if (line.find("%%EndPageSetup") != std::string::npos) {
            inData = false;
        }
    }

    // If no layers found, create a single default layer
    if (info.layers.empty() && foundBBox) {
        AILayerInfo layer;
        layer.name = info.title.empty() ? "Background" : info.title;
        layer.width = info.width;
        layer.height = info.height;
        info.layers.push_back(layer);
    }

    return foundBBox;
}

bool AIImporter::parsePDFHeader(FILE* f, AIFileInfo& info) {
    fseek(f, 0, SEEK_SET);

    // Read PDF header and look for AI-specific metadata
    char buffer[4096];
    size_t totalRead = 0;
    std::string content;

    // Read first 64KB for header parsing
    size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[bytesRead] = '\0';
    content.assign(buffer, bytesRead);

    // Look for /MediaBox [llx lly urx ury]
    size_t mbPos = content.find("/MediaBox");
    if (mbPos != std::string::npos) {
        size_t start = content.find('[', mbPos);
        size_t end = content.find(']', start);
        if (start != std::string::npos && end != std::string::npos) {
            float llx = 0, lly = 0, urx = 0, ury = 0;
            std::string mb = content.substr(start + 1, end - start - 1);
            if (sscanf(mb.c_str(), "%f %f %f %f", &llx, &lly, &urx, &ury) == 4) {
                info.width = static_cast<int>(std::round(urx - llx));
                info.height = static_cast<int>(std::round(ury - lly));
            }
        }
    }

    // Look for /Title
    size_t titlePos = content.find("/Title");
    if (titlePos != std::string::npos) {
        size_t start = content.find('(', titlePos);
        size_t end = content.find(')', start);
        if (start != std::string::npos && end != std::string::npos) {
            info.title = content.substr(start + 1, end - start - 1);
        }
    }

    // Look for /Author
    size_t authorPos = content.find("/Author");
    if (authorPos != std::string::npos) {
        size_t start = content.find('(', authorPos);
        size_t end = content.find(')', start);
        if (start != std::string::npos && end != std::string::npos) {
            info.creator = content.substr(start + 1, end - start - 1);
        }
    }

    // Look for page count and layer structure
    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    long searchStart = std::max(0L, fileSize - 65536); // Last 64KB
    fseek(f, searchStart, SEEK_SET);
    bytesRead = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[bytesRead] = '\0';
    std::string tail(buffer, bytesRead);

    // Look for /Page object references
    size_t pagePos = 0;
    int pageCount = 0;
    while ((pagePos = tail.find("/Type /Page", pagePos)) != std::string::npos) {
        pageCount++;
        pagePos += 11;
    }

    // PDF/AI files embed layers as separate pages or as optional content groups
    // For now, create a default layer entry
    if (info.width > 0 && info.height > 0) {
        AILayerInfo layer;
        layer.name = info.title.empty() ? "Background" : info.title;
        layer.width = info.width;
        layer.height = info.height;
        layer.visible = true;
        info.layers.push_back(layer);
    }

    return info.width > 0 && info.height > 0;
}

bool AIImporter::parsePDFBody(FILE* f, AIFileInfo& info) {
    // Stub for full PDF body parsing - extract embedded raster images
    return false;
}

bool AIImporter::importLayers(const std::string& path, AIFileInfo& info) {
    if (info.width == 0) {
        info = parseFile(path);
    }

    // For PDF-based AI, attempt to extract raster data from streams
    if (info.isPDF) {
        FILE* f = fopen(path.c_str(), "rb");
        if (!f) return false;

        // Scan for image XObjects in the PDF
        fseek(f, 0, SEEK_END);
        long fileSize = ftell(f);
        fseek(f, 0, SEEK_SET);

        // Read in chunks looking for image stream markers
        const size_t chunkSize = 8192;
        std::vector<char> chunk(chunkSize);
        long pos = 0;

        while (pos < fileSize) {
            fseek(f, pos, SEEK_SET);
            size_t read = fread(chunk.data(), 1, chunkSize, f);
            if (read == 0) break;

            std::string chunkStr(chunk.data(), read);

            // Look for /Subtype /Image
            size_t imgPos = 0;
            while ((imgPos = chunkStr.find("/Subtype /Image", imgPos)) != std::string::npos) {
                // Found an embedded image - for now just note it exists
                imgPos += 15;
            }

            pos += read - 16; // overlap for boundary matches
        }

        fclose(f);
    }

    // EPS: populate pixel data if raster images are embedded
    for (auto& layer : info.layers) {
        if (layer.pixelData.empty() && layer.width > 0 && layer.height > 0) {
            // Create placeholder RGBA buffer (transparent)
            layer.pixelData.resize(layer.width * layer.height * 4, 0);
        }
    }

    return true;
}

std::string AIImporter::readLine(FILE* f) {
    std::string line;
    int c;
    while ((c = fgetc(f)) != EOF && c != '\n') {
        if (c != '\r') {
            line += static_cast<char>(c);
        }
    }
    return line;
}

void AIImporter::trimInPlace(std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        s.clear();
        return;
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    s = s.substr(start, end - start + 1);
}

} // namespace FreeEffect
