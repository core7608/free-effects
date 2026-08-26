#pragma once

#include "../math/matrix4.h"
#include "../timeline/composition.h"
#include "blend_modes.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace FreeEffect {

struct PixelBuffer {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> data;
    
    void resize(int w, int h) {
        width = w;
        height = h;
        data.resize(w * h * 4, 0);
    }
    
    uint8_t* pixelAt(int x, int y) {
        int offset = (y * width + x) * 4;
        return data.data() + offset;
    }
    
    const uint8_t* pixelAt(int x, int y) const {
        int offset = (y * width + x) * 4;
        return data.data() + offset;
    }
};

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    PixelBuffer renderFrame(const Composition& comp, double timeInSeconds);
    
    void setQuality(int quality) { m_quality = quality; }
    int getQuality() const { return m_quality; }
    
    void setResolution(int width, int height) { m_renderWidth = width; m_renderHeight = height; }

    void setBlendMode(BlendModeType mode) { m_currentBlendMode = mode; }
    BlendModeType getBlendMode() const { return m_currentBlendMode; }

    void setCameraMatrices(const float view[16], const float projection[16]);
    void setUse3D(bool use) { m_use3D = use; }
    bool getUse3D() const { return m_use3D; }

private:
    void clearBuffer(PixelBuffer& buffer, Color bgColor);
    void compositeLayer(PixelBuffer& target, const Layer& layer, double time, const Composition& comp);
    void applyEffects(PixelBuffer& buffer, const Layer& layer, double time);
    void blendNormal(PixelBuffer& target, const PixelBuffer& source, int x, int y, double opacity, BlendModeType mode);
    void applyAdjustmentLayers(PixelBuffer& buffer, const Composition& comp, double time, int startLayerIndex);
    Mat4 computeWorldTransform(const Layer& layer, const Composition& comp);

    int m_quality = 100;
    int m_renderWidth = 1920;
    int m_renderHeight = 1080;

    BlendModeType m_currentBlendMode = BlendModeType::Normal;
    bool m_use3D = false;
    float m_viewMatrix[16] = {};
    float m_projectionMatrix[16] = {};
};

} // namespace FreeEffect
