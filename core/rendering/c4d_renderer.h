#pragma once

#include "renderer.h"
#include "scene_renderer.h"
#include "../timeline/composition.h"
#include "../tools/tool_interface.h"
#include "../io/obj_importer.h"
#include <memory>
#include <string>
#include <vector>

namespace FreeEffect {

struct TextStyle {
    std::string fontFamily = "Arial";
    float fontSize = 48.0f;
    bool bold = false;
    bool italic = false;
    Color fillColor{1.0, 1.0, 1.0, 1.0};
    Color strokeColor{0.0, 0.0, 0.0, 0.0};
    float strokeWidth = 0.0f;
};

class C4DRenderer {
public:
    PixelBuffer renderExtrudedText(const std::string& text, const TextStyle& style,
                                    double depth, double bevel, double bevelRadius,
                                    const Camera& camera, const std::vector<std::shared_ptr<Light>>& lights);
    
    PixelBuffer renderExtrudedShape(const std::vector<BezierPoint>& path, 
                                     double depth, double bevel, double bevelRadius,
                                     const Camera& camera, const std::vector<std::shared_ptr<Light>>& lights);
    
    PixelBuffer renderModel(const OBJMesh& mesh, const Camera& camera,
                            const std::vector<std::shared_ptr<Light>>& lights);
    
    PixelBuffer renderShadowMap(const Camera& lightCam, const std::vector<RenderObject>& objects);
    
    void applySSAO(PixelBuffer& buffer, const PixelBuffer& depthBuffer, float radius, float intensity);
    
    void applyEnvironmentMapping(PixelBuffer& buffer, const PixelBuffer& envMap, float reflectivity);

private:
    void rasterizeTriangle(const float v0[4], const float v1[4], const float v2[4],
                           const float n0[3], const float n1[3], const float n2[3],
                           const float c0[4], const float c1[4], const float c2[4],
                           PixelBuffer& buffer, float viewProj[16], float* depthBuffer);
    
    struct ExtrudedMesh {
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<int> indices;
    };
    ExtrudedMesh extrudePath(const std::vector<BezierPoint>& path, double depth);
    ExtrudedMesh bevelPath(const std::vector<BezierPoint>& path, double depth, double bevelRadius);

    void buildMeshFromTriangles(const std::vector<float>& verts,
                                const std::vector<float>& norms,
                                const std::vector<int>& idxs,
                                const Camera& camera,
                                const std::vector<std::shared_ptr<Light>>& lights,
                                int width, int height,
                                PixelBuffer& buffer, float* depthBuf);
};

} // namespace FreeEffect
