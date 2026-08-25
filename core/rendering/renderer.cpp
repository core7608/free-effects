#include "renderer.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

Renderer::Renderer() {
}

Renderer::~Renderer() {
}

PixelBuffer Renderer::renderFrame(const Composition& comp, double timeInSeconds) {
    PixelBuffer buffer;
    buffer.resize(comp.getResolution().width, comp.getResolution().height);
    
    clearBuffer(buffer, comp.getBackgroundColor());
    
    const auto& layers = comp.getLayers();
    for (int i = static_cast<int>(layers.size()) - 1; i >= 0; --i) {
        const auto& layer = layers[i];
        if (layer->isVisible() && layer->isActiveAtTime(timeInSeconds)) {
            compositeLayer(buffer, *layer, timeInSeconds, comp);
        }
    }
    
    return buffer;
}

void Renderer::clearBuffer(PixelBuffer& buffer, Color bgColor) {
    uint8_t r = static_cast<uint8_t>(std::clamp(bgColor.r, 0.0, 1.0) * 255.0);
    uint8_t g = static_cast<uint8_t>(std::clamp(bgColor.g, 0.0, 1.0) * 255.0);
    uint8_t b = static_cast<uint8_t>(std::clamp(bgColor.b, 0.0, 1.0) * 255.0);
    uint8_t a = static_cast<uint8_t>(std::clamp(bgColor.a, 0.0, 1.0) * 255.0);
    
    for (int i = 0; i < buffer.width * buffer.height; ++i) {
        int offset = i * 4;
        buffer.data[offset]     = r;
        buffer.data[offset + 1] = g;
        buffer.data[offset + 2] = b;
        buffer.data[offset + 3] = a;
    }
}

void Renderer::compositeLayer(PixelBuffer& target, const Layer& layer, double time, const Composition& comp) {
    double opacity = layer.getOpacity().getValueAtTime(time) / 100.0;
    if (opacity <= 0.0) return;
    
    // For MVP, render solid layers as colored rectangles
    // Real image/video compositing will be added with FFmpeg integration
    if (layer.getType() == LayerType::Solid) {
        double posX = layer.getPosition().getValueAtTime(time);
        double posY = layer.getPosition().getValueAtTime(time);
        double scale = layer.getScale().getValueAtTime(time) / 100.0;
        
        int centerX = static_cast<int>(posX);
        int centerY = static_cast<int>(posY);
        int halfW = static_cast<int>(100 * scale);
        int halfH = static_cast<int>(100 * scale);
        
        for (int y = std::max(0, centerY - halfH); y < std::min(target.height, centerY + halfH); ++y) {
            for (int x = std::max(0, centerX - halfW); x < std::min(target.width, centerX + halfW); ++x) {
                uint8_t* pixel = target.pixelAt(x, y);
                pixel[0] = static_cast<uint8_t>(std::clamp(1.0 * 255.0 * opacity, 0.0, 255.0));
                pixel[1] = static_cast<uint8_t>(std::clamp(1.0 * 255.0 * opacity, 0.0, 255.0));
                pixel[2] = static_cast<uint8_t>(std::clamp(1.0 * 255.0 * opacity, 0.0, 255.0));
                pixel[3] = static_cast<uint8_t>(std::clamp(opacity * 255.0, 0.0, 255.0));
            }
        }
    }
}

void Renderer::blendNormal(PixelBuffer& target, const PixelBuffer& source, int x, int y, double opacity) {
    for (int sy = 0; sy < source.height; ++sy) {
        for (int sx = 0; sx < source.width; ++sx) {
            int tx = x + sx;
            int ty = y + sy;
            if (tx < 0 || tx >= target.width || ty < 0 || ty >= target.height) continue;
            
            const uint8_t* src = source.pixelAt(sx, sy);
            uint8_t* dst = target.pixelAt(tx, ty);
            
            double a = (src[3] / 255.0) * opacity;
            double invA = 1.0 - a;
            
            dst[0] = static_cast<uint8_t>(src[0] * a + dst[0] * invA);
            dst[1] = static_cast<uint8_t>(src[1] * a + dst[1] * invA);
            dst[2] = static_cast<uint8_t>(src[2] * a + dst[2] * invA);
            dst[3] = static_cast<uint8_t>(std::clamp((a + dst[3] / 255.0 * invA) * 255.0, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
