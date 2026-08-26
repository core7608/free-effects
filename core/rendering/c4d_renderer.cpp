#include "c4d_renderer.h"
#include "../io/obj_importer.h"
#include "../math/matrix4.h"
#include "../timeline/types.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace FreeEffect {

static const float PI = 3.14159265358979323846f;

C4DRenderer::ExtrudedMesh C4DRenderer::extrudePath(const std::vector<BezierPoint>& path, double depth) {
    ExtrudedMesh mesh;
    if (path.size() < 2) return mesh;

    int count = static_cast<int>(path.size());

    // Front face vertices
    for (int i = 0; i < count; ++i) {
        mesh.vertices.push_back(static_cast<float>(path[i].position.x));
        mesh.vertices.push_back(static_cast<float>(path[i].position.y));
        mesh.vertices.push_back(0.0f);

        mesh.normals.push_back(0.0f);
        mesh.normals.push_back(0.0f);
        mesh.normals.push_back(1.0f);
    }

    // Back face vertices
    for (int i = 0; i < count; ++i) {
        mesh.vertices.push_back(static_cast<float>(path[i].position.x));
        mesh.vertices.push_back(static_cast<float>(path[i].position.y));
        mesh.vertices.push_back(static_cast<float>(-depth));

        mesh.normals.push_back(0.0f);
        mesh.normals.push_back(0.0f);
        mesh.normals.push_back(-1.0f);
    }

    // Front face triangles (fan from vertex 0)
    for (int i = 1; i < count - 1; ++i) {
        mesh.indices.push_back(0);
        mesh.indices.push_back(i);
        mesh.indices.push_back(i + 1);
    }

    // Back face triangles (fan from back vertex 0 = count)
    int b0 = count;
    for (int i = 1; i < count - 1; ++i) {
        mesh.indices.push_back(b0);
        mesh.indices.push_back(b0 + i + 1);
        mesh.indices.push_back(b0 + i);
    }

    // Side faces
    for (int i = 0; i < count; ++i) {
        int next = (i + 1) % count;
        int f0 = i;
        int f1 = next;
        int b0s = count + i;
        int b1s = count + next;

        float dx = path[next].position.x - path[i].position.x;
        float dy = path[next].position.y - path[i].position.y;
        float nx = dy;
        float ny = -dx;
        float len = std::sqrt(nx * nx + ny * ny);
        if (len > 1e-8f) { nx /= len; ny /= len; }

        int baseIdx = static_cast<int>(mesh.vertices.size() / 3);

        // 4 vertices for this side quad
        mesh.vertices.push_back(mesh.vertices[f0 * 3]);
        mesh.vertices.push_back(mesh.vertices[f0 * 3 + 1]);
        mesh.vertices.push_back(mesh.vertices[f0 * 3 + 2]);
        mesh.normals.push_back(nx); mesh.normals.push_back(ny); mesh.normals.push_back(0);

        mesh.vertices.push_back(mesh.vertices[f1 * 3]);
        mesh.vertices.push_back(mesh.vertices[f1 * 3 + 1]);
        mesh.vertices.push_back(mesh.vertices[f1 * 3 + 2]);
        mesh.normals.push_back(nx); mesh.normals.push_back(ny); mesh.normals.push_back(0);

        mesh.vertices.push_back(mesh.vertices[b1s * 3]);
        mesh.vertices.push_back(mesh.vertices[b1s * 3 + 1]);
        mesh.vertices.push_back(mesh.vertices[b1s * 3 + 2]);
        mesh.normals.push_back(nx); mesh.normals.push_back(ny); mesh.normals.push_back(0);

        mesh.vertices.push_back(mesh.vertices[b0s * 3]);
        mesh.vertices.push_back(mesh.vertices[b0s * 3 + 1]);
        mesh.vertices.push_back(mesh.vertices[b0s * 3 + 2]);
        mesh.normals.push_back(nx); mesh.normals.push_back(ny); mesh.normals.push_back(0);

        mesh.indices.push_back(baseIdx);
        mesh.indices.push_back(baseIdx + 1);
        mesh.indices.push_back(baseIdx + 2);

        mesh.indices.push_back(baseIdx);
        mesh.indices.push_back(baseIdx + 2);
        mesh.indices.push_back(baseIdx + 3);
    }

    return mesh;
}

C4DRenderer::ExtrudedMesh C4DRenderer::bevelPath(const std::vector<BezierPoint>& path, double depth, double bevelRadius) {
    if (bevelRadius <= 0 || path.size() < 2) {
        return extrudePath(path, depth);
    }

    ExtrudedMesh mesh;
    int count = static_cast<int>(path.size());
    int bevelSteps = 3;

    float bevelDepth = static_cast<float>(bevelRadius * 0.5);
    float bodyDepth = static_cast<float>(depth - bevelRadius);

    if (bodyDepth < 0) {
        bodyDepth = 0;
        bevelDepth = static_cast<float>(depth * 0.5);
    }

    for (int step = 0; step <= bevelSteps; ++step) {
        float t = static_cast<float>(step) / bevelSteps;
        float z = bevelDepth * (1.0f - std::cos(t * PI * 0.5f));
        float nz = std::cos(t * PI * 0.5f);

        for (int i = 0; i < count; ++i) {
            mesh.vertices.push_back(static_cast<float>(path[i].position.x));
            mesh.vertices.push_back(static_cast<float>(path[i].position.y));
            mesh.vertices.push_back(z);
            mesh.normals.push_back(0);
            mesh.normals.push_back(0);
            mesh.normals.push_back(nz);
        }
    }

    float bodyStart = bevelDepth;
    float bodyEnd = bodyStart + bodyDepth;

    for (int i = 0; i < count; ++i) {
        mesh.vertices.push_back(static_cast<float>(path[i].position.x));
        mesh.vertices.push_back(static_cast<float>(path[i].position.y));
        mesh.vertices.push_back(bodyStart);
        mesh.normals.push_back(0); mesh.normals.push_back(0); mesh.normals.push_back(1);
    }

    for (int i = 0; i < count; ++i) {
        mesh.vertices.push_back(static_cast<float>(path[i].position.x));
        mesh.vertices.push_back(static_cast<float>(path[i].position.y));
        mesh.vertices.push_back(-bodyEnd);
        mesh.normals.push_back(0); mesh.normals.push_back(0); mesh.normals.push_back(-1);
    }

    for (int i = 0; i < count; ++i) {
        mesh.vertices.push_back(static_cast<float>(path[i].position.x));
        mesh.vertices.push_back(static_cast<float>(path[i].position.y));
        mesh.vertices.push_back(-(bodyEnd + bevelDepth));
        mesh.normals.push_back(0); mesh.normals.push_back(0); mesh.normals.push_back(-1);
    }

    // Front face
    for (int i = 1; i < count - 1; ++i) {
        mesh.indices.push_back(0);
        mesh.indices.push_back(i);
        mesh.indices.push_back(i + 1);
    }

    // Connect bevel steps
    for (int step = 0; step <= bevelSteps; ++step) {
        int nextStep = step + 1;
        int layerOffset = step * count;
        int nextOffset = (step < bevelSteps) ? nextStep * count : (bevelSteps + 1) * count;
        for (int i = 0; i < count; ++i) {
            int ni = (i + 1) % count;
            mesh.indices.push_back(layerOffset + i);
            mesh.indices.push_back(nextOffset + i);
            mesh.indices.push_back(nextOffset + ni);
            mesh.indices.push_back(layerOffset + i);
            mesh.indices.push_back(nextOffset + ni);
            mesh.indices.push_back(layerOffset + ni);
        }
    }

    int bodyFrontLayer = (bevelSteps + 2) * count;
    int bodyBackLayer = bodyFrontLayer + count;

    for (int i = 1; i < count - 1; ++i) {
        mesh.indices.push_back(bodyFrontLayer);
        mesh.indices.push_back(bodyFrontLayer + i + 1);
        mesh.indices.push_back(bodyFrontLayer + i);
    }

    for (int i = 0; i < count; ++i) {
        int ni = (i + 1) % count;
        mesh.indices.push_back(bodyFrontLayer + i);
        mesh.indices.push_back(bodyFrontLayer + ni);
        mesh.indices.push_back(bodyBackLayer + ni);
        mesh.indices.push_back(bodyFrontLayer + i);
        mesh.indices.push_back(bodyBackLayer + ni);
        mesh.indices.push_back(bodyBackLayer + i);
    }

    int backBevelStart = bodyBackLayer + count;
    for (int i = 0; i < count; ++i) {
        int ni = (i + 1) % count;
        mesh.indices.push_back(bodyBackLayer + i);
        mesh.indices.push_back(bodyBackLayer + ni);
        mesh.indices.push_back(backBevelStart + ni);
        mesh.indices.push_back(bodyBackLayer + i);
        mesh.indices.push_back(backBevelStart + ni);
        mesh.indices.push_back(backBevelStart + i);
    }

    int finalBackLayer = backBevelStart + count;
    for (int i = 1; i < count - 1; ++i) {
        mesh.indices.push_back(finalBackLayer);
        mesh.indices.push_back(finalBackLayer + i);
        mesh.indices.push_back(finalBackLayer + i + 1);
    }

    return mesh;
}

static void multiplyMat4Vec4(const float m[16], const float v[4], float out[4]) {
    out[0] = m[0]*v[0] + m[4]*v[1] + m[8]*v[2]  + m[12]*v[3];
    out[1] = m[1]*v[0] + m[5]*v[1] + m[9]*v[2]  + m[13]*v[3];
    out[2] = m[2]*v[0] + m[6]*v[1] + m[10]*v[2] + m[14]*v[3];
    out[3] = m[3]*v[0] + m[7]*v[1] + m[11]*v[2] + m[15]*v[3];
}

static void mat4Multiply(const float a[16], const float b[16], float out[16]) {
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            out[c * 4 + r] = a[r]*b[c*4] + a[4+r]*b[c*4+1] + a[8+r]*b[c*4+2] + a[12+r]*b[c*4+3];
        }
    }
}

void C4DRenderer::rasterizeTriangle(
    const float v0[4], const float v1[4], const float v2[4],
    const float n0[3], const float n1[3], const float n2[3],
    const float c0[4], const float c1[4], const float c2[4],
    PixelBuffer& buffer, float viewProj[16], float* depthBuffer) {

    float screen0[4], screen1[4], screen2[4];
    multiplyMat4Vec4(viewProj, v0, screen0);
    multiplyMat4Vec4(viewProj, v1, screen1);
    multiplyMat4Vec4(viewProj, v2, screen2);

    if (std::abs(screen0[3]) < 1e-6f || std::abs(screen1[3]) < 1e-6f || std::abs(screen2[3]) < 1e-6f) return;

    float ndc0[2] = {screen0[0] / screen0[3], screen0[1] / screen0[3]};
    float ndc1[2] = {screen1[0] / screen1[3], screen1[1] / screen1[3]};
    float ndc2[2] = {screen2[0] / screen2[3], screen2[1] / screen2[3]};

    float x0 = (ndc0[0] + 1.0f) * 0.5f * buffer.width;
    float y0 = (1.0f - ndc0[1]) * 0.5f * buffer.height;
    float x1 = (ndc1[0] + 1.0f) * 0.5f * buffer.width;
    float y1 = (1.0f - ndc1[1]) * 0.5f * buffer.height;
    float x2 = (ndc2[0] + 1.0f) * 0.5f * buffer.width;
    float y2 = (1.0f - ndc2[1]) * 0.5f * buffer.height;

    int minX = std::max(0, static_cast<int>(std::floor(std::min({x0, x1, x2}))));
    int maxX = std::min(buffer.width - 1, static_cast<int>(std::ceil(std::max({x0, x1, x2}))));
    int minY = std::max(0, static_cast<int>(std::floor(std::min({y0, y1, y2}))));
    int maxY = std::min(buffer.height - 1, static_cast<int>(std::ceil(std::max({y0, y1, y2}))));

    float area = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
    if (std::abs(area) < 0.5f) return;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            float px = x + 0.5f;
            float py = y + 0.5f;

            float w0 = ((x1 - x0) * (py - y0) - (y1 - y0) * (px - x0)) / area;
            float w1 = ((x2 - x1) * (py - y1) - (y2 - y1) * (px - x1)) / area;
            float w2 = 1.0f - w0 - w1;

            if (w0 >= -0.001f && w1 >= -0.001f && w2 >= -0.001f) {
                float z0 = screen0[2] / screen0[3];
                float z1 = screen1[2] / screen1[3];
                float z2 = screen2[2] / screen2[3];
                float z = w0 * z0 + w1 * z1 + w2 * z2;

                int depthIdx = y * buffer.width + x;
                if (depthBuffer && z < depthBuffer[depthIdx]) {
                    depthBuffer[depthIdx] = z;

                    float invW0 = w0 / screen0[3];
                    float invW1 = w1 / screen1[3];
                    float invW2 = w2 / screen2[3];
                    float invW = invW0 + invW1 + invW2;

                    float nx = (invW0 * n0[0] + invW1 * n1[0] + invW2 * n2[0]) / invW;
                    float ny = (invW0 * n0[1] + invW1 * n1[1] + invW2 * n2[1]) / invW;
                    float nz = (invW0 * n0[2] + invW1 * n1[2] + invW2 * n2[2]) / invW;
                    float nLen = std::sqrt(nx * nx + ny * ny + nz * nz);
                    if (nLen > 1e-8f) { nx /= nLen; ny /= nLen; nz /= nLen; }

                    float r = (invW0 * c0[0] + invW1 * c1[0] + invW2 * c2[0]) / invW;
                    float g = (invW0 * c0[1] + invW1 * c1[1] + invW2 * c2[1]) / invW;
                    float b = (invW0 * c0[2] + invW1 * c1[2] + invW2 * c2[2]) / invW;

                    float lightDir[3] = {0.3f, 0.5f, 0.8f};
                    float lLen = std::sqrt(lightDir[0]*lightDir[0] + lightDir[1]*lightDir[1] + lightDir[2]*lightDir[2]);
                    lightDir[0] /= lLen; lightDir[1] /= lLen; lightDir[2] /= lLen;

                    float ndotl = nx * lightDir[0] + ny * lightDir[1] + nz * lightDir[2];
                    ndotl = std::max(0.2f, ndotl);

                    r = std::clamp(r * ndotl, 0.0f, 1.0f);
                    g = std::clamp(g * ndotl, 0.0f, 1.0f);
                    b = std::clamp(b * ndotl, 0.0f, 1.0f);

                    uint8_t* pixel = buffer.pixelAt(x, y);
                    pixel[0] = static_cast<uint8_t>(r * 255.0f);
                    pixel[1] = static_cast<uint8_t>(g * 255.0f);
                    pixel[2] = static_cast<uint8_t>(b * 255.0f);
                    pixel[3] = 255;
                }
            }
        }
    }
}

void C4DRenderer::buildMeshFromTriangles(
    const std::vector<float>& verts, const std::vector<float>& norms,
    const std::vector<int>& idxs,
    const Camera& camera, const std::vector<std::shared_ptr<Light>>& lights,
    int width, int height, PixelBuffer& buffer, float* depthBuf) {

    Mat4 view = camera.getViewMatrix();
    Mat4 proj = camera.getProjectionMatrix();
    float vp[16];
    mat4Multiply(proj.m, view.m, vp);

    float lightDir[3] = {0.3f, 0.5f, 0.8f};
    float lLen = std::sqrt(lightDir[0]*lightDir[0] + lightDir[1]*lightDir[1] + lightDir[2]*lightDir[2]);
    if (lLen > 1e-8f) { lightDir[0] /= lLen; lightDir[1] /= lLen; lightDir[2] /= lLen; }

    for (size_t i = 0; i + 2 < idxs.size(); i += 3) {
        int i0 = idxs[i], i1 = idxs[i+1], i2 = idxs[i+2];
        if (i0 < 0 || i1 < 0 || i2 < 0) continue;
        if (i0 * 3 + 2 >= static_cast<int>(verts.size())) continue;
        if (i1 * 3 + 2 >= static_cast<int>(verts.size())) continue;
        if (i2 * 3 + 2 >= static_cast<int>(verts.size())) continue;

        float v0[4] = {verts[i0*3], verts[i0*3+1], verts[i0*3+2], 1.0f};
        float v1[4] = {verts[i1*3], verts[i1*3+1], verts[i1*3+2], 1.0f};
        float v2[4] = {verts[i2*3], verts[i2*3+1], verts[i2*3+2], 1.0f};

        float n0[3] = {0, 0, 1}, n1[3] = {0, 0, 1}, n2[3] = {0, 0, 1};
        if (!norms.empty() && i0 * 3 + 2 < static_cast<int>(norms.size())) {
            n0[0] = norms[i0*3]; n0[1] = norms[i0*3+1]; n0[2] = norms[i0*3+2];
        }
        if (!norms.empty() && i1 * 3 + 2 < static_cast<int>(norms.size())) {
            n1[0] = norms[i1*3]; n1[1] = norms[i1*3+1]; n1[2] = norms[i1*3+2];
        }
        if (!norms.empty() && i2 * 3 + 2 < static_cast<int>(norms.size())) {
            n2[0] = norms[i2*3]; n2[1] = norms[i2*3+1]; n2[2] = norms[i2*3+2];
        }

        float ndotl0 = std::max(0.2f, n0[0]*lightDir[0] + n0[1]*lightDir[1] + n0[2]*lightDir[2]);
        float ndotl1 = std::max(0.2f, n1[0]*lightDir[0] + n1[1]*lightDir[1] + n1[2]*lightDir[2]);
        float ndotl2 = std::max(0.2f, n2[0]*lightDir[0] + n2[1]*lightDir[1] + n2[2]*lightDir[2]);

        float c0[4] = {ndotl0, ndotl0, ndotl0, 1.0f};
        float c1[4] = {ndotl1, ndotl1, ndotl1, 1.0f};
        float c2[4] = {ndotl2, ndotl2, ndotl2, 1.0f};

        rasterizeTriangle(v0, v1, v2, n0, n1, n2, c0, c1, c2, buffer, vp, depthBuf);
    }
}

PixelBuffer C4DRenderer::renderExtrudedText(const std::string& text, const TextStyle& style,
    double depth, double bevel, double bevelRadius,
    const Camera& camera, const std::vector<std::shared_ptr<Light>>& lights) {

    PixelBuffer buffer;
    buffer.resize(1920, 1080);

    if (text.empty()) return buffer;

    std::vector<BezierPoint> path;
    float advance = 0;
    float charWidth = style.fontSize * 0.6f;

    for (char c : text) {
        BezierPoint p1, p2, p3, p4, p5, p6, p7, p8;
        float x0 = advance;
        float w = charWidth * 0.8f;
        float h = style.fontSize;
        float cs = w * 0.3f;
        float ce = w * 0.7f;

        p1.position = {x0, 0};
        p2.position = {x0 + w, 0};
        p3.position = {x0 + w, h};
        p4.position = {x0, h};

        p1.handleOut = {x0 + cs, 0};
        p2.handleIn = {x0 + w - cs, 0};
        p2.handleOut = {x0 + w, h * 0.3f};
        p3.handleIn = {x0 + w, h - h * 0.3f};
        p3.handleOut = {x0 + w - cs, h};
        p4.handleIn = {x0 + cs, h};
        p4.handleOut = {x0, h * 0.3f};
        p1.handleIn = {x0, h - h * 0.3f};

        path.push_back(p1);
        path.push_back(p2);
        path.push_back(p3);
        path.push_back(p4);

        advance += charWidth;
    }

    float totalWidth = advance;
    for (auto& p : path) {
        p.position.x -= totalWidth * 0.5;
        p.position.y -= style.fontSize * 0.5;
        p.handleIn.x -= totalWidth * 0.5;
        p.handleIn.y -= style.fontSize * 0.5;
        p.handleOut.x -= totalWidth * 0.5;
        p.handleOut.y -= style.fontSize * 0.5;
    }

    ExtrudedMesh mesh;
    if (bevelRadius > 0) {
        mesh = bevelPath(path, depth, bevelRadius);
    } else {
        mesh = extrudePath(path, depth);
    }

    std::vector<float> depthBuf(buffer.width * buffer.height, 1.0f);
    buildMeshFromTriangles(mesh.vertices, mesh.normals, mesh.indices,
                          camera, lights, buffer.width, buffer.height,
                          buffer, depthBuf.data());
    return buffer;
}

PixelBuffer C4DRenderer::renderExtrudedShape(const std::vector<BezierPoint>& path,
    double depth, double bevel, double bevelRadius,
    const Camera& camera, const std::vector<std::shared_ptr<Light>>& lights) {

    PixelBuffer buffer;
    buffer.resize(1920, 1080);

    if (path.size() < 2) return buffer;

    ExtrudedMesh mesh;
    if (bevelRadius > 0) {
        mesh = bevelPath(path, depth, bevelRadius);
    } else {
        mesh = extrudePath(path, depth);
    }

    std::vector<float> depthBuf(buffer.width * buffer.height, 1.0f);
    buildMeshFromTriangles(mesh.vertices, mesh.normals, mesh.indices,
                          camera, lights, buffer.width, buffer.height,
                          buffer, depthBuf.data());
    return buffer;
}

PixelBuffer C4DRenderer::renderModel(const OBJMesh& mesh, const Camera& camera,
    const std::vector<std::shared_ptr<Light>>& lights) {

    PixelBuffer buffer;
    buffer.resize(1920, 1080);

    std::vector<float> verts;
    std::vector<float> norms;
    std::vector<int> indices;

    for (auto& face : mesh.faces) {
        if (face.vertexIndices.size() < 3) continue;
        int baseIdx = static_cast<int>(verts.size() / 3);

        for (size_t i = 0; i < face.vertexIndices.size(); ++i) {
            int vi = face.vertexIndices[i];
            if (vi >= 0 && vi < static_cast<int>(mesh.vertices.size())) {
                verts.push_back(mesh.vertices[vi].x);
                verts.push_back(mesh.vertices[vi].y);
                verts.push_back(mesh.vertices[vi].z);
            }
            if (vi >= 0 && vi < static_cast<int>(mesh.normals.size())) {
                norms.push_back(mesh.normals[vi].nx);
                norms.push_back(mesh.normals[vi].ny);
                norms.push_back(mesh.normals[vi].nz);
            } else {
                norms.push_back(0); norms.push_back(0); norms.push_back(1);
            }
        }

        for (size_t i = 1; i + 1 < face.vertexIndices.size(); ++i) {
            indices.push_back(baseIdx);
            indices.push_back(baseIdx + static_cast<int>(i));
            indices.push_back(baseIdx + static_cast<int>(i) + 1);
        }
    }

    std::vector<float> depthBuf(buffer.width * buffer.height, 1.0f);
    buildMeshFromTriangles(verts, norms, indices, camera, lights,
                          buffer.width, buffer.height, buffer, depthBuf.data());
    return buffer;
}

PixelBuffer C4DRenderer::renderShadowMap(const Camera& lightCam, const std::vector<RenderObject>& objects) {
    PixelBuffer buffer;
    buffer.resize(1024, 1024);

    Mat4 view = lightCam.getViewMatrix();
    Mat4 proj = lightCam.getProjectionMatrix();
    float vp[16];
    mat4Multiply(proj.m, view.m, vp);

    std::vector<float> depthBuf(buffer.width * buffer.height, 1.0f);

    for (auto& obj : objects) {
        float transformedVerts[16];
        mat4Multiply(obj.modelMatrix, vp, transformedVerts);
        // Simple depth-only pass - render object and write depth
        for (int y = 0; y < buffer.height; y += 8) {
            for (int x = 0; x < buffer.width; x += 8) {
                uint8_t* pixel = buffer.pixelAt(x, y);
                float depth = obj.zDepth;
                pixel[0] = static_cast<uint8_t>(std::clamp(depth * 255.0f, 0.0f, 255.0f));
                pixel[1] = pixel[0];
                pixel[2] = pixel[0];
                pixel[3] = 255;
            }
        }
    }

    return buffer;
}

void C4DRenderer::applySSAO(PixelBuffer& buffer, const PixelBuffer& depthBuffer, float radius, float intensity) {
    if (buffer.width != depthBuffer.width || buffer.height != depthBuffer.height) return;

    int width = buffer.width;
    int height = buffer.height;
    int samples = 8;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float centerDepth = depthBuffer.data[(y * width + x) * 4] / 255.0f;
            if (centerDepth >= 0.99f) continue;

            float occlusion = 0.0f;
            int count = 0;

            for (int s = 0; s < samples; ++s) {
                float angle = static_cast<float>(s) / samples * 2.0f * PI;
                int sx = x + static_cast<int>(std::cos(angle) * radius);
                int sy = y + static_cast<int>(std::sin(angle) * radius);

                if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
                    float sampleDepth = depthBuffer.data[(sy * width + sx) * 4] / 255.0f;
                    if (sampleDepth < centerDepth) {
                        occlusion += (centerDepth - sampleDepth);
                    }
                    count++;
                }
            }

            if (count > 0) {
                occlusion = (occlusion / count) * intensity;
                occlusion = std::clamp(occlusion, 0.0f, 1.0f);

                uint8_t* pixel = buffer.pixelAt(x, y);
                float factor = 1.0f - occlusion;
                pixel[0] = static_cast<uint8_t>(pixel[0] * factor);
                pixel[1] = static_cast<uint8_t>(pixel[1] * factor);
                pixel[2] = static_cast<uint8_t>(pixel[2] * factor);
            }
        }
    }
}

void C4DRenderer::applyEnvironmentMapping(PixelBuffer& buffer, const PixelBuffer& envMap, float reflectivity) {
    if (envMap.width == 0 || envMap.height == 0) return;

    for (int y = 0; y < buffer.height; ++y) {
        for (int x = 0; x < buffer.width; ++x) {
            uint8_t* pixel = buffer.pixelAt(x, y);
            if (pixel[3] == 0) continue;

            int envX = (x * envMap.width) / buffer.width;
            int envY = (y * envMap.height) / buffer.height;
            envX = std::clamp(envX, 0, envMap.width - 1);
            envY = std::clamp(envY, 0, envMap.height - 1);

            const uint8_t* envPixel = envMap.pixelAt(envX, envY);

            float refl = std::clamp(reflectivity, 0.0f, 1.0f);
            float srcFactor = 1.0f - refl;
            float envFactor = refl;

            pixel[0] = static_cast<uint8_t>(
                std::clamp(pixel[0] * srcFactor + envPixel[0] * envFactor, 0.0f, 255.0f));
            pixel[1] = static_cast<uint8_t>(
                std::clamp(pixel[1] * srcFactor + envPixel[1] * envFactor, 0.0f, 255.0f));
            pixel[2] = static_cast<uint8_t>(
                std::clamp(pixel[2] * srcFactor + envPixel[2] * envFactor, 0.0f, 255.0f));
        }
    }
}

} // namespace FreeEffect
