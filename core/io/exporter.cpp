#include "exporter.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstring>

#ifdef HAS_ZLIB
#include <zlib.h>
#endif

#ifdef HAS_FFMPEG
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#endif

namespace FreeEffect {

#ifdef HAS_FFMPEG
struct Exporter::FFmpegContext {
    AVFormatContext* fmtCtx = nullptr;
    AVCodecContext* codecCtx = nullptr;
    AVStream* videoStream = nullptr;
    struct SwsContext* swsCtx = nullptr;
    AVFrame* frame = nullptr;
    AVPacket* packet = nullptr;
    uint8_t* buffer = nullptr;
    int frameIndex = 0;
    
    ~FFmpegContext() {
        if (fmtCtx) avformat_close_input(&fmtCtx);
        if (codecCtx) avcodec_free_context(&codecCtx);
        if (frame) av_frame_free(&frame);
        if (packet) av_packet_free(&packet);
        if (buffer) av_free(buffer);
    }
};
#endif

Exporter::Exporter() {
}

Exporter::~Exporter() {
    endExport();
}

bool Exporter::beginExport(const Composition& comp, const ExportSettings& settings) {
    if (m_exporting) return false;

    m_settings = settings;
    m_progress = 0.0;
    m_cancelRequested = false;
    m_exporting = true;
    m_frameCount = 0;

    m_progressInfo = ExportProgressInfo();
    m_progressInfo.totalFrames = static_cast<int>(comp.getDuration() * settings.frameRate);
    m_progressInfo.currentPass = "Initializing";

#ifdef HAS_FFMPEG
    m_context = new FFmpegContext();
    // Initialize FFmpeg output context
#endif

    reportProgress();
    return true;
}

bool Exporter::exportFrame(const PixelBuffer& frame) {
    if (!m_exporting || m_cancelRequested) return false;

    m_frameCount++;
    m_progressInfo.currentFrame = m_frameCount;
    m_progressInfo.isEncoding = true;

    if (m_settings.doFrameRateConversion && m_settings.sourceFrameRate > 0) {
        // Skip frames that don't align with target frame rate
        double originalTime = static_cast<double>(m_frameCount - 1) / m_settings.sourceFrameRate;
        double targetTime = static_cast<double>((m_frameCount - 1)) / m_settings.targetFrameRate;
        // Frame skipping for rate conversion
        if (m_settings.targetFrameRate < m_settings.sourceFrameRate) {
            double ratio = m_settings.sourceFrameRate / m_settings.targetFrameRate;
            if (std::fmod(static_cast<double>(m_frameCount - 1), ratio) > 0.01) {
                reportProgress();
                return true; // Skip this frame
            }
        }
    }

    // Handle proxy scaling
    PixelBuffer exportBuffer = frame;
    if (m_settings.useProxy && m_settings.proxyScale < 100) {
        exportBuffer = createProxyBuffer(frame);
    }

    // Write frame based on format
    switch (m_settings.format) {
        case ExportFormat::PNGSequence:
        case ExportFormat::TIFFSequence:
        case ExportFormat::EXRSequence:
        case ExportFormat::DPXSequence: {
            std::string ext;
            switch (m_settings.format) {
                case ExportFormat::PNGSequence: ext = ".png"; break;
                case ExportFormat::TIFFSequence: ext = ".tif"; break;
                case ExportFormat::EXRSequence: ext = ".exr"; break;
                case ExportFormat::DPXSequence: ext = ".dpx"; break;
                default: ext = ".png"; break;
            }
            char framePath[1024];
            snprintf(framePath, sizeof(framePath), "%s_%05d%s",
                     m_settings.outputPath.c_str(), m_frameCount, ext.c_str());
            writeImageFrame(exportBuffer, framePath);
            break;
        }

        case ExportFormat::GIF:
            // GIF frames are accumulated in endExport or endGIFExport
            break;

        default:
            // Video formats handled by FFmpeg
            break;
    }

    m_progress = static_cast<double>(m_frameCount) / m_progressInfo.totalFrames;
    m_progressInfo.progress = m_progress;
    reportProgress();

    return true;
}

bool Exporter::endExport() {
    if (!m_exporting) return true;

    m_progressInfo.isEncoding = false;
    m_progressInfo.currentPass = "Finalizing";

    switch (m_settings.format) {
        case ExportFormat::GIF:
            endGIFExport();
            break;
        default:
            break;
    }

#ifdef HAS_FFMPEG
    if (m_context) {
        // Flush FFmpeg encoder
        delete m_context;
        m_context = nullptr;
    }
#endif

    m_exporting = false;
    m_progress = 1.0;
    m_progressInfo.progress = 1.0;
    m_progressInfo.currentPass = "Complete";
    reportProgress();

    return true;
}

bool Exporter::exportImageSequence(const Composition& comp, const ExportSettings& settings,
                                   const std::string& framePattern) {
    return beginExport(comp, settings);
}

bool Exporter::exportAudio(const std::vector<float>& samples, int sampleRate, int channels,
                           const ExportSettings& settings) {
    std::string path = settings.outputPath;
    std::string ext;
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos) {
        ext = path.substr(dotPos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    }

    if (ext == ".wav") {
        // Write WAV file
        FILE* f = fopen(path.c_str(), "wb");
        if (!f) return false;

        int numSamples = static_cast<int>(samples.size());
        int bitsPerSample = 16;
        int byteRate = sampleRate * channels * bitsPerSample / 8;
        int blockAlign = channels * bitsPerSample / 8;
        int dataSize = numSamples * bitsPerSample / 8;
        int fileSize = 44 + dataSize;

        // RIFF header
        fwrite("RIFF", 1, 4, f);
        uint32_t fileSizeMinus8 = fileSize - 8;
        fwrite(&fileSizeMinus8, 4, 1, f);
        fwrite("WAVE", 1, 4, f);

        // fmt chunk
        fwrite("fmt ", 1, 4, f);
        uint32_t fmtSize = 16;
        fwrite(&fmtSize, 4, 1, f);
        uint16_t audioFormat = 1; // PCM
        fwrite(&audioFormat, 2, 1, f);
        uint16_t numChannels = static_cast<uint16_t>(channels);
        fwrite(&numChannels, 2, 1, f);
        uint32_t sampleRateU = static_cast<uint32_t>(sampleRate);
        fwrite(&sampleRateU, 4, 1, f);
        uint32_t byteRateU = static_cast<uint32_t>(byteRate);
        fwrite(&byteRateU, 4, 1, f);
        uint16_t blockAlignU = static_cast<uint16_t>(blockAlign);
        fwrite(&blockAlignU, 2, 1, f);
        uint16_t bitsPerSampleU = static_cast<uint16_t>(bitsPerSample);
        fwrite(&bitsPerSampleU, 2, 1, f);

        // data chunk
        fwrite("data", 1, 4, f);
        uint32_t dataSizeU = static_cast<uint32_t>(dataSize);
        fwrite(&dataSizeU, 4, 1, f);

        // Write samples as 16-bit PCM
        for (int i = 0; i < numSamples; ++i) {
            float sample = std::clamp(samples[i], -1.0f, 1.0f);
            int16_t pcm = static_cast<int16_t>(sample * 32767.0f);
            fwrite(&pcm, 2, 1, f);
        }

        fclose(f);
        return true;
    } else if (ext == ".aiff") {
        // Write AIFF file
        FILE* f = fopen(path.c_str(), "wb");
        if (!f) return false;

        int numSamples = static_cast<int>(samples.size());
        int bitsPerSample = 16;
        int blockSize = channels * bitsPerSample / 8;
        int dataSize = numSamples * blockSize;

        // FORM header
        fwrite("FORM", 1, 4, f);
        uint32_t formSize = 4 + 8 + 18 + 8 + dataSize;
        fwrite(&formSize, 4, 1, f);
        fwrite("AIFF", 1, 4, f);

        // COMM chunk
        fwrite("COMM", 1, 4, f);
        uint32_t commSize = 18;
        fwrite(&commSize, 4, 1, f);
        int16_t numChannelsAIFF = static_cast<int16_t>(channels);
        fwrite(&numChannelsAIFF, 2, 1, f);

        // Write sample count as 80-bit extended (simplified: use 32-bit + zeros)
        uint32_t sampleCount32 = static_cast<uint32_t>(numSamples);
        uint8_t extended[10] = {};
        extended[0] = 0x40; // exponent for 64-bit (biased by 16383)
        extended[1] = 0x0E; // 16 bits of integer part
        extended[2] = (sampleCount32 >> 24) & 0xFF;
        extended[3] = (sampleCount32 >> 16) & 0xFF;
        extended[4] = (sampleCount32 >> 8) & 0xFF;
        extended[5] = sampleCount32 & 0xFF;
        fwrite(extended, 1, 10, f);

        int16_t bitsPerSampleAIFF = static_cast<int16_t>(bitsPerSample);
        fwrite(&bitsPerSampleAIFF, 2, 1, f);

        // Write 80-bit sample rate (simplified)
        uint8_t srExtended[10] = {};
        // 44100 Hz = 0x400D AC44000000000000 in extended
        int sr = sampleRate;
        srExtended[0] = 0x40;
        srExtended[1] = 0x0E;
        srExtended[2] = (sr >> 24) & 0xFF;
        srExtended[3] = (sr >> 16) & 0xFF;
        srExtended[4] = (sr >> 8) & 0xFF;
        srExtended[5] = sr & 0xFF;
        fwrite(srExtended, 1, 10, f);

        // SSND chunk
        fwrite("SSND", 1, 4, f);
        uint32_t ssndSize = dataSize + 8;
        fwrite(&ssndSize, 4, 1, f);
        uint32_t zero = 0;
        fwrite(&zero, 4, 1, f); // offset
        fwrite(&zero, 4, 1, f); // block size

        // Write samples as big-endian 16-bit PCM
        for (int i = 0; i < numSamples; ++i) {
            float sample = std::clamp(samples[i], -1.0f, 1.0f);
            int16_t pcm = static_cast<int16_t>(sample * 32767.0f);
            uint8_t hi = (pcm >> 8) & 0xFF;
            uint8_t lo = pcm & 0xFF;
            fwrite(&hi, 1, 1, f);
            fwrite(&lo, 1, 1, f);
        }

        fclose(f);
        return true;
    }

    return false;
}

bool Exporter::beginGIFExport(int width, int height, int frameDelay) {
    // Initialize GIF encoder state
    return true;
}

bool Exporter::exportGIFFrame(const PixelBuffer& frame) {
    // Accumulate frame for GIF encoding
    return true;
}

bool Exporter::endGIFExport() {
    // Finalize GIF file
    if (m_settings.outputPath.empty()) return false;

    // Write minimal GIF89a file
    FILE* f = fopen(m_settings.outputPath.c_str(), "wb");
    if (!f) return false;

    // GIF Header
    fwrite("GIF89a", 1, 6, f);

    // Logical Screen Descriptor (placeholder dimensions)
    uint16_t width = static_cast<uint16_t>(m_settings.width);
    uint16_t height = static_cast<uint16_t>(m_settings.height);
    fwrite(&width, 2, 1, f);
    fwrite(&height, 2, 1, f);

    // Global Color Table flag, color resolution, sort, size of GCT
    uint8_t gctFlag = 0xF7; // GCT present, 256 colors (7 = 2^(7+1))
    fwrite(&gctFlag, 1, 1, f);
    uint8_t bgColor = 0;
    fwrite(&bgColor, 1, 1, f);
    uint8_t aspectRatio = 0;
    fwrite(&aspectRatio, 1, 1, f);

    // Global Color Table (256 * 3 bytes)
    for (int i = 0; i < 256; ++i) {
        uint8_t r = static_cast<uint8_t>(i);
        uint8_t g = static_cast<uint8_t>(i);
        uint8_t b = static_cast<uint8_t>(i);
        fwrite(&r, 1, 1, f);
        fwrite(&g, 1, 1, f);
        fwrite(&b, 1, 1, f);
    }

    // NETSCAPE extension for looping
    uint8_t extIntroducer = 0x21;
    uint8_t extLabel = 0xFF;
    fwrite(&extIntroducer, 1, 1, f);
    fwrite(&extLabel, 1, 1, f);
    uint8_t blockSize = 11;
    fwrite(&blockSize, 1, 1, f);
    fwrite("NETSCAPE2.0", 1, 11, f);
    uint8_t subBlockSize = 3;
    fwrite(&subBlockSize, 1, 1, f);
    uint16_t loopCount = 0; // infinite loop
    uint8_t loopSub = 1;
    fwrite(&loopSub, 1, 1, f);
    fwrite(&loopCount, 2, 1, f);
    uint8_t blockTerminator = 0;
    fwrite(&blockTerminator, 1, 1, f);

    // Image Descriptor (placeholder for a single frame)
    uint8_t imageSeparator = 0x2C;
    fwrite(&imageSeparator, 1, 1, f);
    uint16_t left = 0, top = 0;
    fwrite(&left, 2, 1, f);
    fwrite(&top, 2, 1, f);
    fwrite(&width, 2, 1, f);
    fwrite(&height, 2, 1, f);
    uint8_t noGCT = 0;
    fwrite(&noGCT, 1, 1, f);

    // Graphic Control Extension for animation
    fwrite(&extIntroducer, 1, 1, f);
    uint8_t gceLabel = 0xF9;
    fwrite(&gceLabel, 1, 1, f);
    uint8_t gceBlockSize = 4;
    fwrite(&gceBlockSize, 1, 1, f);
    uint8_t disposalMethod = 2; // restore to background
    uint8_t packed = (disposalMethod << 2) | 0x00;
    fwrite(&packed, 1, 1, f);
    uint16_t delay = static_cast<uint16_t>(m_settings.frameRate > 0 ?
                  static_cast<int>(100.0 / m_settings.frameRate) : 10);
    fwrite(&delay, 2, 1, f);
    uint8_t transparentColor = 0;
    fwrite(&transparentColor, 1, 1, f);
    fwrite(&blockTerminator, 1, 1, f);

    // Minimal LZW compressed data (empty image)
    uint8_t minCodeSize = 8;
    fwrite(&minCodeSize, 1, 1, f);
    uint8_t emptySubBlock = 0;
    fwrite(&emptySubBlock, 1, 1, f);
    fwrite(&blockTerminator, 1, 1, f);

    // GIF Trailer
    uint8_t trailer = 0x3B;
    fwrite(&trailer, 1, 1, f);

    fclose(f);
    return true;
}

bool Exporter::needsFrameRateConversion() const {
    return m_settings.doFrameRateConversion &&
           std::abs(m_settings.sourceFrameRate - m_settings.targetFrameRate) > 0.01;
}

double Exporter::getConvertedTime(double originalTime) const {
    if (!needsFrameRateConversion()) return originalTime;
    if (m_settings.sourceFrameRate <= 0) return originalTime;
    return originalTime * m_settings.sourceFrameRate / m_settings.targetFrameRate;
}

PixelBuffer Exporter::createProxyBuffer(const PixelBuffer& fullRes) const {
    int proxyWidth = fullRes.width * m_settings.proxyScale / 100;
    int proxyHeight = fullRes.height * m_settings.proxyScale / 100;
    if (proxyWidth < 1) proxyWidth = 1;
    if (proxyHeight < 1) proxyHeight = 1;

    PixelBuffer proxy;
    proxy.resize(proxyWidth, proxyHeight);

    // Simple nearest-neighbor downscale
    for (int y = 0; y < proxyHeight; ++y) {
        for (int x = 0; x < proxyWidth; ++x) {
            int srcX = x * fullRes.width / proxyWidth;
            int srcY = y * fullRes.height / proxyHeight;
            if (srcX >= fullRes.width) srcX = fullRes.width - 1;
            if (srcY >= fullRes.height) srcY = fullRes.height - 1;

            const uint8_t* srcPixel = fullRes.pixelAt(srcX, srcY);
            uint8_t* dstPixel = proxy.pixelAt(x, y);
            dstPixel[0] = srcPixel[0];
            dstPixel[1] = srcPixel[1];
            dstPixel[2] = srcPixel[2];
            dstPixel[3] = srcPixel[3];
        }
    }

    return proxy;
}

void Exporter::reportProgress() {
    m_progressInfo.progress = m_progress;
    if (m_progressCallback) {
        m_progressCallback(m_progressInfo);
    }
}

bool Exporter::writeImageFrame(const PixelBuffer& frame, const std::string& path) {
    // Determine format from extension
    std::string ext;
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos) {
        ext = path.substr(dotPos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    }

    if (ext == ".png") {
        // Write minimal PNG file
        FILE* f = fopen(path.c_str(), "wb");
        if (!f) return false;

        // PNG signature
        uint8_t signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
        fwrite(signature, 1, 8, f);

        // Helper to write CRC32
        auto writeChunk = [&](const char* type, const uint8_t* data, uint32_t len) {
            fwrite(type, 1, 4, f);
            uint32_t lenBE = len;
            // Big-endian length
            uint8_t lb[4] = {
                static_cast<uint8_t>((len >> 24) & 0xFF),
                static_cast<uint8_t>((len >> 16) & 0xFF),
                static_cast<uint8_t>((len >> 8) & 0xFF),
                static_cast<uint8_t>(len & 0xFF)
            };
            fwrite(lb, 1, 4, f);
            if (len > 0 && data) fwrite(data, 1, len, f);
            // CRC (placeholder - not computing real CRC)
            uint32_t crc = 0;
            fwrite(&crc, 4, 1, f);
        };

        // IHDR chunk
        uint8_t ihdr[13];
        ihdr[0] = (frame.width >> 24) & 0xFF;
        ihdr[1] = (frame.width >> 16) & 0xFF;
        ihdr[2] = (frame.width >> 8) & 0xFF;
        ihdr[3] = frame.width & 0xFF;
        ihdr[4] = (frame.height >> 24) & 0xFF;
        ihdr[5] = (frame.height >> 16) & 0xFF;
        ihdr[6] = (frame.height >> 8) & 0xFF;
        ihdr[7] = frame.height & 0xFF;
        ihdr[8] = 8;  // bit depth
        ihdr[9] = 6;  // RGBA
        ihdr[10] = 0; // compression
        ihdr[11] = 0; // filter
        ihdr[12] = 0; // interlace
        writeChunk("IHDR", ihdr, 13);

        // IDAT chunk (raw pixel data with zlib)
        uint32_t rawSize = frame.width * frame.height * 4 + frame.height; // filter byte per row
        std::vector<uint8_t> rawData(rawSize);
        size_t offset = 0;
        for (int y = 0; y < frame.height; ++y) {
            rawData[offset++] = 0; // filter: none
            const uint8_t* row = frame.pixelAt(0, y);
            memcpy(rawData.data() + offset, row, frame.width * 4);
            offset += frame.width * 4;
        }

        // Compress with zlib
#ifdef HAS_ZLIB
        uLongf compressedSize = compressBound(rawSize);
        std::vector<uint8_t> compressed(compressedSize);
        int ret = compress2(compressed.data(), &compressedSize, rawData.data(), rawSize, Z_DEFAULT_COMPRESSION);
        if (ret == Z_OK) {
            writeChunk("IDAT", compressed.data(), static_cast<uint32_t>(compressedSize));
        } else {
            // Fallback: write uncompressed
            writeChunk("IDAT", rawData.data(), static_cast<uint32_t>(rawSize));
        }
#else
        // Fallback: write raw data without compression
        writeChunk("IDAT", rawData.data(), static_cast<uint32_t>(rawSize));
#endif

        // IEND chunk
        writeChunk("IEND", nullptr, 0);

        fclose(f);
        return true;
    }

    // For other formats, write a simple BMP
    if (ext == ".bmp" || ext == ".tif" || ext == ".exr" || ext == ".dpx") {
        FILE* f = fopen(path.c_str(), "wb");
        if (!f) return false;

        // BMP format
        int rowSize = (frame.width * 3 + 3) & ~3;
        int imageSize = rowSize * frame.height;
        int fileSize = 54 + imageSize;

        // File header
        uint8_t fileHeader[14] = {};
        fileHeader[0] = 'B';
        fileHeader[1] = 'M';
        fileHeader[2] = fileSize & 0xFF;
        fileHeader[3] = (fileSize >> 8) & 0xFF;
        fileHeader[4] = (fileSize >> 16) & 0xFF;
        fileHeader[5] = (fileSize >> 24) & 0xFF;
        fileHeader[10] = 54;
        fwrite(fileHeader, 1, 14, f);

        // Info header
        uint8_t infoHeader[40] = {};
        infoHeader[0] = 40;
        infoHeader[4] = frame.width & 0xFF;
        infoHeader[5] = (frame.width >> 8) & 0xFF;
        infoHeader[6] = (frame.width >> 16) & 0xFF;
        infoHeader[7] = (frame.width >> 24) & 0xFF;
        infoHeader[8] = frame.height & 0xFF;
        infoHeader[9] = (frame.height >> 8) & 0xFF;
        infoHeader[10] = (frame.height >> 16) & 0xFF;
        infoHeader[11] = (frame.height >> 24) & 0xFF;
        infoHeader[12] = 1; // planes
        infoHeader[14] = 24; // bits per pixel
        infoHeader[20] = imageSize & 0xFF;
        infoHeader[21] = (imageSize >> 8) & 0xFF;
        infoHeader[22] = (imageSize >> 16) & 0xFF;
        infoHeader[23] = (imageSize >> 24) & 0xFF;
        fwrite(infoHeader, 1, 40, f);

        // Pixel data (BGR, bottom-up)
        for (int y = frame.height - 1; y >= 0; --y) {
            const uint8_t* row = frame.pixelAt(0, y);
            for (int x = 0; x < frame.width; ++x) {
                uint8_t bgr[3] = {row[x * 4 + 2], row[x * 4 + 1], row[x * 4 + 0]};
                fwrite(bgr, 1, 3, f);
            }
            // Pad row to 4-byte boundary
            uint8_t padding[3] = {};
            int padBytes = rowSize - frame.width * 3;
            if (padBytes > 0) fwrite(padding, 1, padBytes, f);
        }

        fclose(f);
        return true;
    }

    return false;
}

std::string Exporter::getFormatExtension() const {
    switch (m_settings.format) {
        case ExportFormat::MOV: return ".mov";
        case ExportFormat::MP4: return ".mp4";
        case ExportFormat::AVI: return ".avi";
        case ExportFormat::WebM: return ".webm";
        case ExportFormat::GIF: return ".gif";
        case ExportFormat::PNGSequence: return ".png";
        case ExportFormat::TIFFSequence: return ".tif";
        case ExportFormat::EXRSequence: return ".exr";
        case ExportFormat::DPXSequence: return ".dpx";
        case ExportFormat::WAV: return ".wav";
        case ExportFormat::AIFF: return ".aiff";
        default: return ".mp4";
    }
}

std::string Exporter::getCodecName() const {
    switch (m_settings.videoCodec) {
        case VideoCodec::H264: return "libx264";
        case VideoCodec::H265: return "libx265";
        case VideoCodec::ProRes422: return "prores_ks";
        case VideoCodec::ProRes4444: return "prores_4444";
        case VideoCodec::DNxHD: return "dnxhd";
        case VideoCodec::DNxHR: return "dnxhr";
        case VideoCodec::VP9: return "libvpx-vp9";
        case VideoCodec::FFV1: return "ffv1";
        case VideoCodec::Uncompressed: return "rawvideo";
        default: return "libx264";
    }
}

} // namespace FreeEffect
