#include "psd_importer.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <zlib.h>

namespace FreeEffect {

uint16_t PSDImporter::readU16BE(FILE* f) {
    uint8_t buf[2];
    if (fread(buf, 1, 2, f) != 2) return 0;
    return static_cast<uint16_t>((buf[0] << 8) | buf[1]);
}

uint32_t PSDImporter::readU32BE(FILE* f) {
    uint8_t buf[4];
    if (fread(buf, 1, 4, f) != 4) return 0;
    return static_cast<uint32_t>((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
}

void PSDImporter::skipBytes(FILE* f, long n) {
    fseek(f, n, SEEK_CUR);
}

PSDFileInfo PSDImporter::parseHeader(const std::string& path) {
    PSDFileInfo info;
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return info;

    readPSDHeader(f, info);
    fclose(f);
    return info;
}

bool PSDImporter::readPSDHeader(FILE* f, PSDFileInfo& info) {
    fseek(f, 0, SEEK_SET);

    // Signature: "8BPS"
    char sig[4];
    if (fread(sig, 1, 4, f) != 4) return false;
    if (memcmp(sig, "8BPS", 4) != 0) return false;

    // Version: always 1
    uint16_t version = readU16BE(f);
    if (version != 1) return false;

    // Reserved: 6 bytes
    skipBytes(f, 6);

    // Channels
    info.channels = readU16BE(f);

    // Height (rows)
    info.height = static_cast<int>(readU32BE(f));

    // Width (columns)
    info.width = static_cast<int>(readU32BE(f));

    // Bits per channel
    info.bitDepth = readU16BE(f);

    // Color mode: 0=Bitmap, 1=Grayscale, 2=Indexed, 3=RGB, 4=CMYK, 7=Multichannel, 8=Duotone, 9=Lab
    info.colorMode = readU16BE(f);

    // Skip color mode data section
    uint32_t colorModeDataLen = readU32BE(f);
    skipBytes(f, colorModeDataLen);

    // Skip image resources section
    uint32_t imageResourcesLen = readU32BE(f);
    skipBytes(f, imageResourcesLen);

    // Skip layer and mask info section
    uint32_t layerMaskInfoLen = readU32BE(f);
    long layerSectionEnd = ftell(f) + layerMaskInfoLen;

    // Read layers if layer data present
    if (layerMaskInfoLen > 0) {
        readPSDLayers(f, info);
    }

    // Make sure we're at the right position
    fseek(f, layerSectionEnd, SEEK_SET);

    return true;
}

bool PSDImporter::readPSDLayers(FILE* f, PSDFileInfo& info) {
    // Layer count (signed int16)
    int16_t layerCountRaw = static_cast<int16_t>(readU16BE(f));
    int layerCount = std::abs(layerCountRaw);
    if (layerCount == 0) return true;

    // Read layer info section
    // Each layer record is at least 34 bytes header
    for (int i = 0; i < layerCount; ++i) {
        PSDLayerInfo layer;

        long recordStart = ftell(f);

        // Top, Left, Bottom, Right
        int32_t top = static_cast<int32_t>(readU32BE(f));
        int32_t left = static_cast<int32_t>(readU32BE(f));
        int32_t bottom = static_cast<int32_t>(readU32BE(f));
        int32_t right = static_cast<int32_t>(readU32BE(f));

        layer.x = left;
        layer.y = top;
        layer.width = right - left;
        layer.height = bottom - top;

        // Channels
        uint16_t numChannels = readU16BE(f);

        // Skip channel image data info (6 bytes each: id(2) + data length(4))
        skipBytes(f, numChannels * 6);

        // Blend mode signature: "8BIM"
        char blendSig[4];
        if (fread(blendSig, 1, 4, f) != 4) return false;
        if (memcmp(blendSig, "8BIM", 4) != 0) return false;

        // Blend mode key (4 bytes)
        char blendKey[4];
        if (fread(blendKey, 1, 4, f) != 4) return false;
        // Map blend mode key to enum
        if (memcmp(blendKey, "norm", 4) == 0) layer.blendMode = 0;
        else if (memcmp(blendKey, "dark", 4) == 0) layer.blendMode = 1;
        else if (memcmp(blendKey, "mul ", 4) == 0) layer.blendMode = 2;
        else if (memcmp(blendKey, "litr", 4) == 0) layer.blendMode = 3;
        else if (memcmp(blendKey, "scrn", 4) == 0) layer.blendMode = 4;
        else if (memcmp(blendKey, "over", 4) == 0) layer.blendMode = 5;
        else layer.blendMode = 0;

        // Opacity: 0-255
        uint8_t opacityByte = 0;
        if (fread(&opacityByte, 1, 1, f) != 1) return false;
        layer.opacity = (opacityByte / 255.0) * 100.0;

        // Clipping: 0=base, 1=non-base
        uint8_t clipping = 0;
        if (fread(&clipping, 1, 1, f) != 1) return false;

        // Flags
        uint8_t flags = 0;
        if (fread(&flags, 1, 1, f) != 1) return false;
        layer.visible = (flags & 0x02) == 0;

        // Filler byte
        skipBytes(f, 1);

        // Extra data length
        uint32_t extraDataLen = readU32BE(f);
        long extraDataStart = ftell(f);

        // Layer mask data
        uint32_t maskDataLen = readU32BE(f);
        skipBytes(f, maskDataLen);

        // Layer blending ranges
        uint32_t blendRangesLen = readU32BE(f);
        skipBytes(f, blendRangesLen);

        // Layer name: Pascal string padded to 4 bytes
        uint8_t nameLen = 0;
        if (fread(&nameLen, 1, 1, f) != 1) return false;
        char nameBuf[256] = {};
        size_t toRead = std::min(static_cast<size_t>(nameLen), sizeof(nameBuf) - 1);
        if (fread(nameBuf, 1, toRead, f) != toRead) return false;
        nameBuf[nameLen] = '\0';
        layer.name = std::string(nameBuf);

        // Read additional layer records (unicode name, effects, etc.)
        long remaining = extraDataLen - (ftell(f) - extraDataStart);
        if (remaining > 0) {
            // Look for unicode layer name
            long savedPos = ftell(f);
            bool foundUnicode = false;
            long searchEnd = extraDataStart + extraDataLen;

            while (ftell(f) < searchEnd - 4) {
                char tag[4];
                if (fread(tag, 1, 4, f) != 4) break;

                if (memcmp(tag, "luni", 4) == 0) {
                    // Unicode layer name
                    uint32_t uniNameLen = readU32BE(f);
                    std::wstring unicodeName;
                    for (uint32_t ch = 0; ch < uniNameLen; ++ch) {
                        uint16_t c = readU16BE(f);
                        if (c > 0 && c < 128) {
                            unicodeName += static_cast<wchar_t>(c);
                        }
                    }
                    if (!unicodeName.empty()) {
                        layer.name = std::string(unicodeName.begin(), unicodeName.end());
                    }
                    foundUnicode = true;
                    break;
                } else if (memcmp(tag, "lclr", 4) == 0) {
                    skipBytes(f, 2);
                } else if (memcmp(tag, "lmfx", 4) == 0) {
                    uint32_t fxLen = readU32BE(f);
                    skipBytes(f, fxLen);
                } else if (memcmp(tag, "lfx2", 4) == 0) {
                    uint32_t fx2Len = readU32BE(f);
                    skipBytes(f, fx2Len);
                } else if (memcmp(tag, "fxid", 4) == 0) {
                    skipBytes(f, 4);
                } else if (memcmp(tag, "Patr", 4) == 0) {
                    uint32_t patLen = readU32BE(f);
                    skipBytes(f, patLen);
                } else if (memcmp(tag, "lnk2", 4) == 0) {
                    uint32_t lnkLen = readU32BE(f);
                    skipBytes(f, lnkLen);
                } else if (memcmp(tag, "lnk3", 4) == 0) {
                    uint32_t lnk3Len = readU32BE(f);
                    skipBytes(f, lnk3Len);
                } else {
                    // Unknown tag, rewind and skip
                    fseek(f, savedPos + 1, SEEK_SET);
                }
            }

            if (!foundUnicode) {
                fseek(f, extraDataStart + extraDataLen, SEEK_SET);
            }
        }

        info.layers.push_back(std::move(layer));

        // Make sure we're at the right position
        fseek(f, extraDataStart + extraDataLen, SEEK_SET);
    }

    return true;
}

bool PSDImporter::importLayers(const std::string& path, PSDFileInfo& info) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;

    // If header not parsed yet, parse it
    if (info.width == 0) {
        readPSDHeader(f, info);
    } else {
        // Jump to the image data section (after color mode, resources, layer/mask)
        fseek(f, 0, SEEK_SET);
        fseek(f, 4, SEEK_SET);  // signature
        fseek(f, 2, SEEK_CUR);  // version
        fseek(f, 6, SEEK_CUR);  // reserved
        fseek(f, 2, SEEK_CUR);  // channels
        fseek(f, 4, SEEK_CUR);  // height
        fseek(f, 4, SEEK_CUR);  // width
        fseek(f, 2, SEEK_CUR);  // bit depth
        fseek(f, 2, SEEK_CUR);  // color mode
        uint32_t cmdLen = readU32BE(f);
        skipBytes(f, cmdLen);
        uint32_t irLen = readU32BE(f);
        skipBytes(f, irLen);
        uint32_t lmiLen = readU32BE(f);
        skipBytes(f, lmiLen);
    }

    // Read the image data section
    uint16_t compression = readU16BE(f);

    int pixelCount = info.width * info.height;
    int channelCount = info.channels;
    int bytesPerSample = (info.bitDepth == 16) ? 2 : 1;
    int bytesPerPixel = channelCount * bytesPerSample;
    int expectedSize = pixelCount * bytesPerPixel;

    if (compression == 0) {
        // Raw data
        std::vector<uint8_t> rawData(expectedSize);
        size_t bytesRead = fread(rawData.data(), 1, expectedSize, f);
        if (bytesRead < expectedSize) {
            rawData.resize(bytesRead);
        }
        fclose(f);

        // Convert raw planar data to RGBA
        for (auto& layer : info.layers) {
            layer.pixelData.resize(info.width * info.height * 4, 255);
            // Fill with flattened composite for now
            for (int y = 0; y < info.height && y < layer.height; ++y) {
                for (int x = 0; x < info.width && x < layer.width; ++x) {
                    int dstIdx = (y * info.width + x) * 4;
                    if (compression == 0 && channelCount >= 3) {
                        int srcR = (y * info.width + x) * bytesPerPixel;
                        if (srcR + 2 < static_cast<int>(rawData.size())) {
                            layer.pixelData[dstIdx + 0] = rawData[srcR + 0]; // R
                            layer.pixelData[dstIdx + 1] = rawData[srcR + 1]; // G
                            layer.pixelData[dstIdx + 2] = rawData[srcR + 2]; // B
                            layer.pixelData[dstIdx + 3] = (channelCount > 3) ? rawData[srcR + 3] : 255;
                        }
                    }
                }
            }
        }
        return true;
    } else if (compression == 1) {
        // RLE compressed (PackBits)
        for (auto& layer : info.layers) {
            layer.pixelData.resize(info.width * info.height * 4, 255);

            for (int ch = 0; ch < channelCount && ch < 4; ++ch) {
                // Read scan line byte counts
                std::vector<uint16_t> lineByteCounts(info.height);
                for (int row = 0; row < info.height; ++row) {
                    lineByteCounts[row] = readU16BE(f);
                }

                // Decompress each scan line
                for (int row = 0; row < info.height; ++row) {
                    int bytesRemaining = lineByteCounts[row];
                    long scanLineStart = ftell(f);
                    long scanLineEnd = scanLineStart + bytesRemaining;

                    int x = 0;
                    while (bytesRemaining > 0 && x < info.width) {
                        uint8_t header = 0;
                        if (fread(&header, 1, 1, f) != 1) break;
                        bytesRemaining--;

                        if (header >= 128) {
                            // Repeat
                            int count = 257 - header;
                            uint8_t value = 0;
                            if (fread(&value, 1, 1, f) != 1) break;
                            bytesRemaining--;
                            for (int c = 0; c < count && x < info.width; ++c, ++x) {
                                int dstIdx = (row * info.width + x) * 4;
                                layer.pixelData[dstIdx + ch] = value;
                                if (ch == 0) {
                                    layer.pixelData[dstIdx + 3] = (channelCount > 3) ? 0 : 255;
                                }
                            }
                        } else {
                            // Literal
                            int count = header + 1;
                            for (int c = 0; c < count && x < info.width && bytesRemaining > 0; ++c, ++x) {
                                uint8_t value = 0;
                                if (fread(&value, 1, 1, f) != 1) break;
                                bytesRemaining--;
                                int dstIdx = (row * info.width + x) * 4;
                                layer.pixelData[dstIdx + ch] = value;
                            }
                        }
                    }
                    // Skip to end of scan line
                    fseek(f, scanLineEnd, SEEK_SET);
                }
            }

            // Set alpha
            if (channelCount < 4) {
                for (int i = 0; i < pixelCount; ++i) {
                    layer.pixelData[i * 4 + 3] = 255;
                }
            }
        }
    } else if (compression == 2) {
        // ZIP without prediction
        uint32_t compressedLen = readU32BE(f);
        std::vector<uint8_t> compressed(compressedLen);
        if (fread(compressed.data(), 1, compressedLen, f) != compressedLen) {
            fclose(f);
            return false;
        }

        std::vector<uint8_t> decompressed;
        if (!decompressData(compressed, decompressed, expectedSize)) {
            fclose(f);
            return false;
        }

        for (auto& layer : info.layers) {
            layer.pixelData.resize(pixelCount * 4, 255);
            for (int i = 0; i < pixelCount && i * bytesPerPixel + 2 < static_cast<int>(decompressed.size()); ++i) {
                layer.pixelData[i * 4 + 0] = decompressed[i * bytesPerPixel + 0];
                layer.pixelData[i * 4 + 1] = (channelCount > 1) ? decompressed[i * bytesPerPixel + 1] : decompressed[i * bytesPerPixel + 0];
                layer.pixelData[i * 4 + 2] = (channelCount > 2) ? decompressed[i * bytesPerPixel + 2] : decompressed[i * bytesPerPixel + 0];
                layer.pixelData[i * 4 + 3] = (channelCount > 3) ? decompressed[i * bytesPerPixel + 3] : 255;
            }
        }
    } else if (compression == 3) {
        // ZIP with prediction
        uint32_t compressedLen = readU32BE(f);
        std::vector<uint8_t> compressed(compressedLen);
        if (fread(compressed.data(), 1, compressedLen, f) != compressedLen) {
            fclose(f);
            return false;
        }

        std::vector<uint8_t> decompressed;
        if (!decompressData(compressed, decompressed, expectedSize)) {
            fclose(f);
            return false;
        }

        // Undo prediction (each sample = sample - previous in scanline)
        for (int row = 0; row < info.height; ++row) {
            for (int ch = 0; ch < channelCount; ++ch) {
                int lineStart = (row * channelCount + ch) * info.width;
                if (info.bitDepth == 16) {
                    for (int x = 1; x < info.width; ++x) {
                        int idx = lineStart + x;
                        if (idx * 2 + 1 < static_cast<int>(decompressed.size())) {
                            uint16_t prev = (decompressed[(idx - 1) * 2] << 8) | decompressed[(idx - 1) * 2 + 1];
                            uint16_t cur = (decompressed[idx * 2] << 8) | decompressed[idx * 2 + 1];
                            uint16_t diff = static_cast<uint16_t>(prev + cur);
                            decompressed[idx * 2] = (diff >> 8) & 0xFF;
                            decompressed[idx * 2 + 1] = diff & 0xFF;
                        }
                    }
                } else {
                    for (int x = 1; x < info.width; ++x) {
                        int idx = lineStart + x;
                        if (idx < static_cast<int>(decompressed.size())) {
                            decompressed[idx] = static_cast<uint8_t>(
                                (decompressed[idx - 1] + decompressed[idx]) & 0xFF);
                        }
                    }
                }
            }
        }

        for (auto& layer : info.layers) {
            layer.pixelData.resize(pixelCount * 4, 255);
            for (int i = 0; i < pixelCount && i * bytesPerPixel + 2 < static_cast<int>(decompressed.size()); ++i) {
                layer.pixelData[i * 4 + 0] = decompressed[i * bytesPerPixel + 0];
                layer.pixelData[i * 4 + 1] = (channelCount > 1) ? decompressed[i * bytesPerPixel + 1] : decompressed[i * bytesPerPixel + 0];
                layer.pixelData[i * 4 + 2] = (channelCount > 2) ? decompressed[i * bytesPerPixel + 2] : decompressed[i * bytesPerPixel + 0];
                layer.pixelData[i * 4 + 3] = (channelCount > 3) ? decompressed[i * bytesPerPixel + 3] : 255;
            }
        }
    }

    fclose(f);
    return true;
}

bool PSDImporter::decompressData(const std::vector<uint8_t>& compressed,
                                 std::vector<uint8_t>& decompressed, int expectedSize) {
    decompressed.resize(expectedSize);
    z_stream stream = {};
    stream.next_in = const_cast<Bytef*>(compressed.data());
    stream.avail_in = static_cast<uInt>(compressed.size());
    stream.next_out = decompressed.data();
    stream.avail_out = static_cast<uInt>(decompressed.size());

    int ret = inflateInit(&stream);
    if (ret != Z_OK) return false;

    ret = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);

    if (ret != Z_STREAM_END && ret != Z_OK) {
        decompressed.resize(stream.total_out);
        return stream.total_out > 0;
    }

    decompressed.resize(stream.total_out);
    return true;
}

std::vector<std::string> PSDImporter::getLayerNames(const std::string& path) {
    PSDFileInfo info = parseHeader(path);
    std::vector<std::string> names;
    names.reserve(info.layers.size());
    for (const auto& layer : info.layers) {
        names.push_back(layer.name);
    }
    return names;
}

} // namespace FreeEffect
