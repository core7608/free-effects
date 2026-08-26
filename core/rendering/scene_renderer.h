#pragma once

#include "renderer.h"
#include "../timeline/camera.h"
#include "../timeline/light.h"
#include "../timeline/layer.h"
#include "../math/matrix4.h"
#include <memory>
#include <vector>

namespace FreeEffect {

struct RenderObject {
    int layerIndex = -1;
    float modelMatrix[16];
    PixelBuffer texture;
    double zDepth = 0;
};

class SceneRenderer {
public:
    void setResolution(int w, int h) { m_width = w; m_height = h; }
    void setCamera(std::shared_ptr<Camera> cam) { m_camera = cam; }
    void setLights(const std::vector<std::shared_ptr<Light>>& lights) { m_lights = lights; }

    PixelBuffer renderScene(const Composition& comp, double time);

    void clearDepthBuffer();
    float getDepth(int x, int y) const;
    void setDepth(int x, int y, float depth);

    Color computeLighting(const Vec3& worldPos, const Vec3& normal, const Color& baseColor) const;

    void applyDepthOfField(PixelBuffer& buffer, float focalDistance, float aperture);
    void applyFilmGrain(PixelBuffer& buffer, double amount);
    void applyVignette(PixelBuffer& buffer, double amount);
    void applyChromaticAberration(PixelBuffer& buffer, double amount);

private:
    int m_width = 1920;
    int m_height = 1080;
    std::shared_ptr<Camera> m_camera;
    std::vector<std::shared_ptr<Light>> m_lights;
    std::vector<float> m_depthBuffer;

    void sortObjectsByDepth(std::vector<RenderObject>& objects);
};

} // namespace FreeEffect
