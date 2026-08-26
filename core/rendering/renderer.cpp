#include "renderer.h"
#include "../effects/effect.h"
#include "../timeline/layer_style.h"
#include "../performance/frame_cache.h"
#include "../performance/spatial_grid.h"
#include "../performance/profiler.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>

namespace FreeEffect {

Renderer::Renderer() {
    std::memset(m_viewMatrix, 0, sizeof(m_viewMatrix));
    std::memset(m_projectionMatrix, 0, sizeof(m_projectionMatrix));
}

Renderer::~Renderer() {
}

void Renderer::setCameraMatrices(const float view[16], const float projection[16]) {
    std::memcpy(m_viewMatrix, view, sizeof(m_viewMatrix));
    std::memcpy(m_projectionMatrix, projection, sizeof(m_projectionMatrix));
    m_use3D = true;
}

PixelBuffer Renderer::renderFrame(const Composition& comp, double timeInSeconds) {
    FREEEFFECT_PROFILE_FUNCTION();

    // Try frame cache first
    int compId = 0;
    {
        const char* idStr = comp.getId().c_str();
        compId = static_cast<int>(std::hash<std::string>{}(comp.getId()));
    }

    // Always invalidate cache for this comp+time (composition state may have changed)
    FrameCache::instance().invalidate(timeInSeconds, compId);

    PixelBuffer result;
    {
        int w = comp.getResolution().width;
        int h = comp.getResolution().height;
        result.resize(w, h);
        clearBuffer(result, comp.getBackgroundColor());

        // Build spatial grid for hit-testing
        SpatialGrid spatialGrid;
        spatialGrid.setBounds(0, 0, static_cast<float>(w), static_cast<float>(h), 64);

        const auto& layers = comp.getLayers();
        for (int i = 0; i < static_cast<int>(layers.size()); ++i) {
            const auto& layer = layers[i];
            if (!layer->isVisible() || !layer->isActiveAtTime(timeInSeconds)) continue;
            double posX = layer->getPosition().getValueAtTime(timeInSeconds);
            double posY = layer->getPosition().getValueAtTime(timeInSeconds);
            double scaleX = layer->getScale().getValueAtTime(timeInSeconds) / 100.0;
            double scaleY = layer->getScale().getValueAtTime(timeInSeconds) / 100.0;
            float bw = static_cast<float>(200.0 * scaleX);
            float bh = static_cast<float>(200.0 * scaleY);
            SpatialItem si;
            si.layerIndex = i;
            si.bounds[0] = static_cast<float>(posX) - bw * 0.5f;
            si.bounds[1] = static_cast<float>(posY) - bh * 0.5f;
            si.bounds[2] = bw;
            si.bounds[3] = bh;
            si.zOrder = i;
            spatialGrid.insert(si);
        }

        Mat4 viewProj = Mat4::identity();
        bool hasCamera = false;

        if (comp.getActiveCamera()) {
            viewProj = comp.getActiveCamera()->getViewProjectionMatrix();
            hasCamera = true;
        }

        std::vector<int> sortedIndices;
        sortedIndices.reserve(layers.size());
        for (int i = 0; i < static_cast<int>(layers.size()); ++i) {
            sortedIndices.push_back(i);
        }

        if (hasCamera) {
            std::sort(sortedIndices.begin(), sortedIndices.end(),
                [&](int a, int b) {
                    const auto& la = layers[a];
                    const auto& lb = layers[b];
                    double za = la->is3D() ? la->getTransform3D().position.z : 0.0;
                    double zb = lb->is3D() ? lb->getTransform3D().position.z : 0.0;
                    return za > zb;
                });
        }

        for (int idx : sortedIndices) {
            const auto& layer = layers[idx];
            if (!layer->isVisible() || !layer->isActiveAtTime(timeInSeconds)) continue;
            if (layer->isGuideLayer()) continue;
            if (layer->getType() == LayerType::Adjustment || layer->isAdjustmentLayer()) continue;

            if (layer->is3D() && hasCamera) {
                Mat4 model = computeWorldTransform(*layer, comp);
                Mat4 mvp = Mat4::multiply(viewProj, model);

                Vec3 worldPos(layer->getTransform3D().position.x,
                              layer->getTransform3D().position.y,
                              layer->getTransform3D().position.z);

                auto parent = layer->getParentLayer();
                while (parent) {
                    worldPos = worldPos + Vec3(parent->getTransform3D().position.x,
                                               parent->getTransform3D().position.y,
                                               parent->getTransform3D().position.z);
                    parent = parent->getParentLayer();
                }

                Vec3 screenPos = layer->getTransform3D().projectTo2D(worldPos, viewProj,
                                                 comp.getResolution().width,
                                                 comp.getResolution().height);

                double depth = screenPos.z;
                if (depth < -1.0 || depth > 1.0) continue;
            }

            compositeLayer(result, *layer, timeInSeconds, comp);
        }

        // Apply adjustment layers
        const auto& allLayers = comp.getLayers();
        for (int i = 0; i < static_cast<int>(allLayers.size()); ++i) {
            const auto& layer = allLayers[i];
            if (!layer->isVisible() || !layer->isActiveAtTime(timeInSeconds)) continue;
            if (layer->getType() != LayerType::Adjustment && !layer->isAdjustmentLayer()) continue;
            applyAdjustmentLayers(result, comp, timeInSeconds, i);
        }
    }

    return result;
}

void Renderer::clearBuffer(PixelBuffer& buffer, Color bgColor) {
    FREEEFFECT_PROFILE_SCOPE("clearBuffer");
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

Mat4 Renderer::computeWorldTransform(const Layer& layer, const Composition& comp) {
    FREEEFFECT_PROFILE_SCOPE("computeWorldTransform");
    Mat4 localTransform = Mat4::identity();
    const auto& t3d = layer.getTransform3D();

    localTransform = Mat4::translation(
        t3d.position.x - t3d.anchorPoint.x,
        t3d.position.y - t3d.anchorPoint.y,
        t3d.position.z - t3d.anchorPoint.z);

    Mat4 rx = Mat4::rotationX(t3d.rotation.x);
    Mat4 ry = Mat4::rotationY(t3d.rotation.y);
    Mat4 rz = Mat4::rotationZ(t3d.rotation.z);
    Mat4 r = Mat4::multiply(rz, Mat4::multiply(ry, rx));
    Mat4 s = Mat4::scale(t3d.scale.x, t3d.scale.y, t3d.scale.z);
    localTransform = Mat4::multiply(Mat4::multiply(localTransform, r), s);

    Mat4 world = localTransform;
    auto parent = layer.getParentLayer();
    while (parent) {
        const auto& pt = parent->getTransform3D();
        Mat4 parentLocal = Mat4::translation(
            pt.position.x - pt.anchorPoint.x,
            pt.position.y - pt.anchorPoint.y,
            pt.position.z - pt.anchorPoint.z);
        Mat4 prx = Mat4::rotationX(pt.rotation.x);
        Mat4 pry = Mat4::rotationY(pt.rotation.y);
        Mat4 prz = Mat4::rotationZ(pt.rotation.z);
        Mat4 pr = Mat4::multiply(prz, Mat4::multiply(pry, prx));
        Mat4 ps = Mat4::scale(pt.scale.x, pt.scale.y, pt.scale.z);
        Mat4 parentMat = Mat4::multiply(Mat4::multiply(parentLocal, pr), ps);
        world = Mat4::multiply(parentMat, world);
        parent = parent->getParentLayer();
    }
    return world;
}

void Renderer::compositeLayer(PixelBuffer& target, const Layer& layer, double time, const Composition& comp) {
    FREEEFFECT_PROFILE_SCOPE("compositeLayer");
    double effectiveTime = time;
    if (layer.isTimeRemapEnabled()) {
        effectiveTime = layer.remapTime(time);
    }

    if (layer.getTimeStretch() != 100.0) {
        effectiveTime = (effectiveTime - layer.getStartTime()) * (100.0 / layer.getTimeStretch()) + layer.getStartTime();
    }

    double opacity = layer.getOpacity().getValueAtTime(effectiveTime) / 100.0;
    if (opacity <= 0.0) return;
    
    BlendModeType layerBlend = mapBlendMode(layer.getBlendMode());

    if (layer.getType() == LayerType::Precomp) {
        const std::string& precompId = layer.getPrecompId();
        auto precomp = comp.getPrecompById(precompId);
        if (precomp) {
            PixelBuffer precompBuffer = renderFrame(*precomp, effectiveTime);

            double posX = layer.getPosition().getValueAtTime(effectiveTime);
            double posY = layer.getPosition().getValueAtTime(effectiveTime);
            int offsetX = static_cast<int>(posX);
            int offsetY = static_cast<int>(posY);

            if (layer.getLayerStyle().hasAnyStyle()) {
                layer.getLayerStyle().renderStyles(precompBuffer,
                    precompBuffer.width, precompBuffer.height);
            }

            blendNormal(target, precompBuffer, offsetX, offsetY, opacity, layerBlend);
        }
        return;
    }

    if (layer.getType() == LayerType::Solid) {
        double posX = layer.getPosition().getValueAtTime(effectiveTime);
        double posY = layer.getPosition().getValueAtTime(effectiveTime);
        double scale = layer.getScale().getValueAtTime(effectiveTime) / 100.0;
        
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

    applyEffects(target, layer, effectiveTime);

    if (layer.getLayerStyle().hasAnyStyle()) {
        layer.getLayerStyle().renderStyles(target, target.width, target.height);
    }
}

void Renderer::applyEffects(PixelBuffer& buffer, const Layer& layer, double time) {
    FREEEFFECT_PROFILE_SCOPE("applyEffects");
    const auto& effects = layer.getEffects();
    for (const auto& effect : effects) {
        if (effect && effect->isEnabled()) {
            effect->updateParameterTracks(time);
            effect->apply(buffer, time);
        }
    }
}

void Renderer::blendNormal(PixelBuffer& target, const PixelBuffer& source,
                           int x, int y, double opacity, BlendModeType mode) {
    FREEEFFECT_PROFILE_SCOPE("blendNormal");
    for (int sy = 0; sy < source.height; ++sy) {
        for (int sx = 0; sx < source.width; ++sx) {
            int tx = x + sx;
            int ty = y + sy;
            if (tx < 0 || tx >= target.width || ty < 0 || ty >= target.height) continue;
            
            const uint8_t* src = source.pixelAt(sx, sy);
            uint8_t* dst = target.pixelAt(tx, ty);

            applyBlendMode(mode, src, dst, opacity);
        }
    }
}

void Renderer::applyAdjustmentLayers(PixelBuffer& buffer, const Composition& comp,
                                     double time, int startLayerIndex) {
    FREEEFFECT_PROFILE_SCOPE("applyAdjustmentLayers");
    const auto& layers = comp.getLayers();
    if (startLayerIndex < 0 || startLayerIndex >= static_cast<int>(layers.size())) return;

    const auto& adjLayer = layers[startLayerIndex];
    double effectiveTime = time;
    if (adjLayer->isTimeRemapEnabled()) {
        effectiveTime = adjLayer->remapTime(time);
    }

    if (adjLayer->getTimeStretch() != 100.0) {
        effectiveTime = (effectiveTime - adjLayer->getStartTime()) * (100.0 / adjLayer->getTimeStretch()) + adjLayer->getStartTime();
    }

    double opacity = adjLayer->getOpacity().getValueAtTime(effectiveTime) / 100.0;
    if (opacity <= 0.0) return;

    PixelBuffer original;
    original.resize(buffer.width, buffer.height);
    std::memcpy(original.data.data(), buffer.data.data(), buffer.data.size());

    const auto& effects = adjLayer->getEffects();
    for (const auto& effect : effects) {
        if (effect && effect->isEnabled()) {
            effect->updateParameterTracks(effectiveTime);
            effect->apply(buffer, effectiveTime);
        }
    }

    if (adjLayer->getLayerStyle().hasAnyStyle()) {
        adjLayer->getLayerStyle().renderStyles(buffer, buffer.width, buffer.height);
    }

    if (opacity < 1.0) {
        int pixelCount = buffer.width * buffer.height;
        for (int p = 0; p < pixelCount; ++p) {
            int offset = p * 4;
            double invOpacity = 1.0 - opacity;
            buffer.data[offset]     = static_cast<uint8_t>(
                std::clamp(buffer.data[offset]     * opacity + original.data[offset]     * invOpacity, 0.0, 255.0));
            buffer.data[offset + 1] = static_cast<uint8_t>(
                std::clamp(buffer.data[offset + 1] * opacity + original.data[offset + 1] * invOpacity, 0.0, 255.0));
            buffer.data[offset + 2] = static_cast<uint8_t>(
                std::clamp(buffer.data[offset + 2] * opacity + original.data[offset + 2] * invOpacity, 0.0, 255.0));
            buffer.data[offset + 3] = static_cast<uint8_t>(
                std::clamp(buffer.data[offset + 3] * opacity + original.data[offset + 3] * invOpacity, 0.0, 255.0));
        }
    }
}

} // namespace FreeEffect
