#include "scene_renderer.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <random>

namespace FreeEffect {

PixelBuffer SceneRenderer::renderScene(const Composition& comp, double time) {
    clearDepthBuffer();

    PixelBuffer buffer;
    buffer.resize(m_width, m_height);

    if (m_camera) {
        clearDepthBuffer();
    }

    Renderer renderer;
    renderer.setResolution(m_width, m_height);

    if (m_camera) {
        Mat4 view = m_camera->getViewMatrix();
        Mat4 proj = m_camera->getProjectionMatrix();
        renderer.setCameraMatrices(view.m, proj.m);
        renderer.setUse3D(true);
    }

    buffer = renderer.renderFrame(comp, time);

    return buffer;
}

void SceneRenderer::clearDepthBuffer() {
    m_depthBuffer.assign(m_width * m_height, 1.0f);
}

float SceneRenderer::getDepth(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return 1.0f;
    return m_depthBuffer[y * m_width + x];
}

void SceneRenderer::setDepth(int x, int y, float depth) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;
    m_depthBuffer[y * m_width + x] = depth;
}

Color SceneRenderer::computeLighting(const Vec3& worldPos, const Vec3& normal, const Color& baseColor) const {
    Vec3 N = normal.normalized();
    double totalR = 0, totalG = 0, totalB = 0;

    for (const auto& light : m_lights) {
        if (!light) continue;

        LightType type = light->getLightType();
        const Transform3D& lt = light->getTransform();
        Color lc = light->getColor();
        double intensity = light->getIntensity() / 100.0;

        if (type == LightType::Ambient) {
            totalR += lc.r * intensity;
            totalG += lc.g * intensity;
            totalB += lc.b * intensity;
            continue;
        }

        Vec3 lightPos(lt.position.x, lt.position.y, lt.position.z);
        Vec3 L;

        if (type == LightType::Directional) {
            L = -lightPos.normalized();
        } else {
            L = (lightPos - worldPos).normalized();
        }

        double NdotL = N.dot(L);
        if (NdotL < 0) NdotL = 0;

        if (type == LightType::Spot) {
            Vec3 spotDir(lt.rotation.x, lt.rotation.y, lt.rotation.z);
            spotDir = -spotDir.normalized();
            double angle = std::acos(std::clamp(L.dot(spotDir), -1.0, 1.0));
            double coneRad = light->getConeAngle() * 3.14159265358979323846 / 360.0;
            double feather = light->getConeFeather() / 100.0;

            if (angle > coneRad) continue;
            if (angle > coneRad * (1.0 - feather)) {
                double t = (angle - coneRad * (1.0 - feather)) / (coneRad * feather);
                NdotL *= (1.0 - t);
            }
        }

        double diffuse = NdotL;
        totalR += lc.r * intensity * diffuse;
        totalG += lc.g * intensity * diffuse;
        totalB += lc.b * intensity * diffuse;
    }

    Color result;
    result.r = std::clamp(baseColor.r * totalR, 0.0, 1.0);
    result.g = std::clamp(baseColor.g * totalG, 0.0, 1.0);
    result.b = std::clamp(baseColor.b * totalB, 0.0, 1.0);
    result.a = baseColor.a;
    return result;
}

void SceneRenderer::sortObjectsByDepth(std::vector<RenderObject>& objects) {
    if (!m_camera) return;

    Mat4 view = m_camera->getViewMatrix();

    for (auto& obj : objects) {
        Vec4 pos(obj.modelMatrix[12], obj.modelMatrix[13], obj.modelMatrix[14], 1.0);
        Vec4 viewPos = Mat4::multiplyPoint(view, pos);
        obj.zDepth = -viewPos.z;
    }

    std::sort(objects.begin(), objects.end(),
        [](const RenderObject& a, const RenderObject& b) {
            return a.zDepth > b.zDepth;
        });
}

void SceneRenderer::applyDepthOfField(PixelBuffer& buffer, float focalDistance, float aperture) {
    if (aperture <= 0.0f) return;

    PixelBuffer temp;
    temp.resize(buffer.width, buffer.height);
    std::memcpy(temp.data.data(), buffer.data.data(), buffer.data.size());

    int w = buffer.width;
    int h = buffer.height;
    int maxBlur = static_cast<int>(aperture * 2.0f);
    if (maxBlur > 20) maxBlur = 20;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float depth = getDepth(x, y);
            float diff = std::abs(depth - focalDistance);
            int blurRadius = static_cast<int>(diff * aperture);
            if (blurRadius > maxBlur) blurRadius = maxBlur;
            if (blurRadius <= 0) continue;

            float r = 0, g = 0, b = 0, a = 0;
            int samples = 0;

            for (int ky = -blurRadius; ky <= blurRadius; ++ky) {
                for (int kx = -blurRadius; kx <= blurRadius; ++kx) {
                    int sx = std::clamp(x + kx, 0, w - 1);
                    int sy = std::clamp(y + ky, 0, h - 1);
                    const uint8_t* p = temp.pixelAt(sx, sy);
                    r += p[0];
                    g += p[1];
                    b += p[2];
                    a += p[3];
                    samples++;
                }
            }

            if (samples > 0) {
                float inv = 1.0f / static_cast<float>(samples);
                uint8_t* dst = buffer.pixelAt(x, y);
                dst[0] = static_cast<uint8_t>(std::clamp(r * inv, 0.0f, 255.0f));
                dst[1] = static_cast<uint8_t>(std::clamp(g * inv, 0.0f, 255.0f));
                dst[2] = static_cast<uint8_t>(std::clamp(b * inv, 0.0f, 255.0f));
                dst[3] = static_cast<uint8_t>(std::clamp(a * inv, 0.0f, 255.0f));
            }
        }
    }
}

void SceneRenderer::applyFilmGrain(PixelBuffer& buffer, double amount) {
    if (amount <= 0.0) return;

    static std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    int pixelCount = buffer.width * buffer.height;
    double strength = amount * 255.0 * 0.1;

    for (int p = 0; p < pixelCount; ++p) {
        int offset = p * 4;
        double noise = dist(rng) * strength;

        buffer.data[offset]     = static_cast<uint8_t>(
            std::clamp(buffer.data[offset] + noise, 0.0, 255.0));
        buffer.data[offset + 1] = static_cast<uint8_t>(
            std::clamp(buffer.data[offset + 1] + noise, 0.0, 255.0));
        buffer.data[offset + 2] = static_cast<uint8_t>(
            std::clamp(buffer.data[offset + 2] + noise, 0.0, 255.0));
    }
}

void SceneRenderer::applyVignette(PixelBuffer& buffer, double amount) {
    if (amount <= 0.0) return;

    int w = buffer.width;
    int h = buffer.height;
    float cx = w * 0.5f;
    float cy = h * 0.5f;
    float maxDist = std::sqrt(cx * cx + cy * cy);
    float strength = static_cast<float>(amount) * 2.0f;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float dx = (x - cx) / cx;
            float dy = (y - cy) / cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float vignette = 1.0f - std::min(dist * strength, 1.0f);

            uint8_t* pixel = buffer.pixelAt(x, y);
            pixel[0] = static_cast<uint8_t>(std::clamp(pixel[0] * vignette, 0.0f, 255.0f));
            pixel[1] = static_cast<uint8_t>(std::clamp(pixel[1] * vignette, 0.0f, 255.0f));
            pixel[2] = static_cast<uint8_t>(std::clamp(pixel[2] * vignette, 0.0f, 255.0f));
        }
    }
}

void SceneRenderer::applyChromaticAberration(PixelBuffer& buffer, double amount) {
    if (amount <= 0.0) return;

    int w = buffer.width;
    int h = buffer.height;
    PixelBuffer temp;
    temp.resize(w, h);
    std::memcpy(temp.data.data(), buffer.data.data(), buffer.data.size());

    float shift = static_cast<float>(amount) * 5.0f;
    float cx = w * 0.5f;
    float cy = h * 0.5f;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float dx = (x - cx) / cx;
            float dy = (y - cy) / cy;
            float dist = std::sqrt(dx * dx + dy * dy);

            float rOff = dist * shift;
            float bOff = -dist * shift;

            int rx = std::clamp(static_cast<int>(x + dx * rOff), 0, w - 1);
            int ry = std::clamp(static_cast<int>(y + dy * rOff), 0, h - 1);
            int bx = std::clamp(static_cast<int>(x + dx * bOff), 0, w - 1);
            int by = std::clamp(static_cast<int>(y + dy * bOff), 0, h - 1);

            const uint8_t* rPx = temp.pixelAt(rx, ry);
            const uint8_t* gPx = temp.pixelAt(x, y);
            const uint8_t* bPx = temp.pixelAt(bx, by);

            uint8_t* dst = buffer.pixelAt(x, y);
            dst[0] = rPx[0];
            dst[1] = gPx[1];
            dst[2] = bPx[2];
        }
    }
}

} // namespace FreeEffect
