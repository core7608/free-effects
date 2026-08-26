#include "../math/math_constants.h"
#include "layer_style.h"
#include "../rendering/renderer.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace FreeEffect {

static inline uint8_t toByte(double v) {
    return static_cast<uint8_t>(std::clamp(v * 255.0, 0.0, 255.0));
}

static inline double fromByte(uint8_t v) {
    return v / 255.0;
}

static inline double clamp01(double v) {
    return std::clamp(v, 0.0, 1.0);
}

static inline double blendMultiply(double src, double dst) {
    return src * dst;
}

static inline double blendScreen(double src, double dst) {
    return 1.0 - (1.0 - src) * (1.0 - dst);
}

static inline double blendOverlay(double src, double dst) {
    return dst < 0.5 ? 2.0 * src * dst : 1.0 - 2.0 * (1.0 - src) * (1.0 - dst);
}

static inline double applyBlendMode(int mode, double src, double dst) {
    switch (mode) {
        case 0: return blendMultiply(src, dst);
        case 1: return src + dst;        // Add
        case 2: return dst - src;        // Subtract
        case 8: return blendScreen(src, dst);
        case 6: return blendOverlay(src, dst);
        case 3: return std::min(src, dst);  // Darken
        case 4: return std::max(src, dst);  // Lighten
        case 9: return blendScreen(src, dst); // Screen for highlight
        default: return blendMultiply(src, dst);
    }
}

std::vector<double> LayerStyle::buildAlphaMask(const PixelBuffer& buf, int w, int h) {
    std::vector<double> mask(w * h);
    for (int i = 0; i < w * h; ++i) {
        mask[i] = fromByte(buf.data[i * 4 + 3]);
    }
    return mask;
}

void LayerStyle::boxBlurH(const std::vector<double>& src, std::vector<double>& dst, int w, int h, int radius) {
    if (radius <= 0) { dst = src; return; }
    double invRadius = 1.0 / (2.0 * radius + 1.0);
    for (int y = 0; y < h; ++y) {
        double sum = 0.0;
        for (int x = -radius; x <= radius; ++x) {
            int cx = std::clamp(x, 0, w - 1);
            sum += src[y * w + cx];
        }
        dst[y * w + 0] = sum * invRadius;
        for (int x = 1; x < w; ++x) {
            int addX = std::min(x + radius, w - 1);
            int remX = std::max(x - radius - 1, 0);
            sum += src[y * w + addX] - src[y * w + remX];
            dst[y * w + x] = sum * invRadius;
        }
    }
}

void LayerStyle::boxBlurV(const std::vector<double>& src, std::vector<double>& dst, int w, int h, int radius) {
    if (radius <= 0) { dst = src; return; }
    double invRadius = 1.0 / (2.0 * radius + 1.0);
    for (int x = 0; x < w; ++x) {
        double sum = 0.0;
        for (int y = -radius; y <= radius; ++y) {
            int cy = std::clamp(y, 0, h - 1);
            sum += src[cy * w + x];
        }
        dst[0 * w + x] = sum * invRadius;
        for (int y = 1; y < h; ++y) {
            int addY = std::min(y + radius, h - 1);
            int remY = std::max(y - radius - 1, 0);
            sum += src[addY * w + x] - src[remY * w + x];
            dst[y * w + x] = sum * invRadius;
        }
    }
}

void LayerStyle::gaussianBlur(const std::vector<double>& src, std::vector<double>& dst, int w, int h, double sigma) {
    if (sigma < 0.5) { dst = src; return; }
    int radius = static_cast<int>(std::ceil(sigma * 2.0));
    std::vector<double> temp(w * h);
    boxBlurH(src, temp, w, h, radius);
    boxBlurV(temp, dst, w, h, radius);
    boxBlurH(dst, temp, w, h, radius);
    boxBlurV(temp, dst, w, h, radius);
}

std::vector<double> LayerStyle::dilate(const std::vector<double>& mask, int w, int h, int radius) {
    std::vector<double> result(w * h, 0.0);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double maxVal = 0.0;
            for (int dy = -radius; dy <= radius; ++dy) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    int nx = std::clamp(x + dx, 0, w - 1);
                    int ny = std::clamp(y + dy, 0, h - 1);
                    maxVal = std::max(maxVal, mask[ny * w + nx]);
                }
            }
            result[y * w + x] = maxVal;
        }
    }
    return result;
}

bool LayerStyle::isEdgePixel(const std::vector<double>& mask, int w, int h, int x, int y) {
    double center = mask[y * w + x];
    if (center < 0.01) return false;
    if (x == 0 || y == 0 || x == w - 1 || y == h - 1) return true;
    double left  = mask[y * w + (x - 1)];
    double right = mask[y * w + (x + 1)];
    double up    = mask[(y - 1) * w + x];
    double down  = mask[(y + 1) * w + x];
    return (left < 0.01 || right < 0.01 || up < 0.01 || down < 0.01);
}

bool LayerStyle::hasAnyStyle() const {
    return m_dropShadow.enabled || m_innerShadow.enabled ||
           m_outerGlow.enabled || m_innerGlow.enabled ||
           m_bevelEmboss.enabled || m_stroke.enabled ||
           m_satin.enabled || m_colorOverlay.enabled ||
           m_gradientOverlay.enabled;
}

void LayerStyle::renderStyles(PixelBuffer& buffer, int w, int h) const {
    if (!hasAnyStyle()) return;

    if (m_dropShadow.enabled) renderDropShadow(buffer, w, h);
    if (m_stroke.enabled) renderStroke(buffer, w, h);
    if (m_outerGlow.enabled) renderOuterGlow(buffer, w, h);
    if (m_innerGlow.enabled) renderInnerGlow(buffer, w, h);
    if (m_innerShadow.enabled) renderInnerShadow(buffer, w, h);
    if (m_bevelEmboss.enabled) renderBevelEmboss(buffer, w, h);
    if (m_satin.enabled) renderSatin(buffer, w, h);
    if (m_colorOverlay.enabled) renderColorOverlay(buffer, w, h);
    if (m_gradientOverlay.enabled) renderGradientOverlay(buffer, w, h);
}

void LayerStyle::renderDropShadow(PixelBuffer& buf, int w, int h) const {
    double rad = m_dropShadow.angle * M_PI / 180.0;
    int offX = static_cast<int>(std::round(std::cos(rad) * m_dropShadow.distance));
    int offY = static_cast<int>(std::round(std::sin(rad) * m_dropShadow.distance));

    std::vector<double> alpha = buildAlphaMask(buf, w, h);

    int spreadRadius = static_cast<int>(std::ceil(m_dropShadow.spread));
    if (spreadRadius > 0) {
        alpha = dilate(alpha, w, h, spreadRadius);
    }

    std::vector<double> blurred(w * h);
    gaussianBlur(alpha, blurred, w, h, m_dropShadow.size);

    double opacityFactor = m_dropShadow.opacity / 100.0;
    double sr = m_dropShadow.color.r;
    double sg = m_dropShadow.color.g;
    double sb = m_dropShadow.color.b;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int sx = x - offX;
            int sy = y - offY;
            if (sx < 0 || sx >= w || sy < 0 || sy >= h) continue;

            double shadowAlpha = blurred[sy * w + sx] * opacityFactor;
            if (shadowAlpha < 0.001) continue;

            uint8_t* dst = buf.pixelAt(x, y);
            double da = fromByte(dst[3]);
            double outA = shadowAlpha + da * (1.0 - shadowAlpha);
            if (outA < 0.001) continue;
            double sa = shadowAlpha / outA;
            dst[0] = toByte(clamp01(sr * sa + fromByte(dst[0]) * (1.0 - sa)));
            dst[1] = toByte(clamp01(sg * sa + fromByte(dst[1]) * (1.0 - sa)));
            dst[2] = toByte(clamp01(sb * sa + fromByte(dst[2]) * (1.0 - sa)));
            dst[3] = toByte(clamp01(outA));
        }
    }
}

void LayerStyle::renderInnerShadow(PixelBuffer& buf, int w, int h) const {
    std::vector<double> alpha = buildAlphaMask(buf, w, h);
    std::vector<double> inverted(w * h);
    for (int i = 0; i < w * h; ++i) {
        inverted[i] = 1.0 - alpha[i];
    }

    std::vector<double> blurred(w * h);
    gaussianBlur(inverted, blurred, w, h, m_innerShadow.size);

    double rad = m_innerShadow.angle * M_PI / 180.0;
    int offX = static_cast<int>(std::round(std::cos(rad) * m_innerShadow.distance));
    int offY = static_cast<int>(std::round(std::sin(rad) * m_innerShadow.distance));

    double opacityFactor = m_innerShadow.opacity / 100.0;
    double sr = m_innerShadow.color.r;
    double sg = m_innerShadow.color.g;
    double sb = m_innerShadow.color.b;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double srcAlpha = alpha[y * w + x];
            if (srcAlpha < 0.01) continue;

            int sx = std::clamp(x + offX, 0, w - 1);
            int sy = std::clamp(y + offY, 0, h - 1);
            double shadowAlpha = blurred[sy * w + sx] * opacityFactor * srcAlpha;
            if (shadowAlpha < 0.001) continue;

            uint8_t* dst = buf.pixelAt(x, y);
            double da = fromByte(dst[3]);
            double sa = clamp01(shadowAlpha);
            double outA = std::min(da + sa * (1.0 - da), 1.0);
            double blend = (sa * (1.0 - da)) / std::max(outA, 0.001);
            dst[0] = toByte(clamp01(fromByte(dst[0]) * (1.0 - blend) + sr * blend));
            dst[1] = toByte(clamp01(fromByte(dst[1]) * (1.0 - blend) + sg * blend));
            dst[2] = toByte(clamp01(fromByte(dst[2]) * (1.0 - blend) + sb * blend));
            dst[3] = toByte(clamp01(outA));
        }
    }
}

void LayerStyle::renderOuterGlow(PixelBuffer& buf, int w, int h) const {
    std::vector<double> alpha = buildAlphaMask(buf, w, h);

    int spreadRadius = static_cast<int>(std::ceil(m_outerGlow.spread * 0.5));
    if (spreadRadius > 0) {
        alpha = dilate(alpha, w, h, spreadRadius);
    }

    std::vector<double> blurred(w * h);
    gaussianBlur(alpha, blurred, w, h, m_outerGlow.size);

    double opacityFactor = m_outerGlow.opacity / 100.0;
    double gr = m_outerGlow.color.r;
    double gg = m_outerGlow.color.g;
    double gb = m_outerGlow.color.b;
    double rangeFactor = m_outerGlow.range / 100.0;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            double srcAlpha = alpha[idx];
            double glowAlpha = std::max(0.0, blurred[idx] - srcAlpha) * opacityFactor;
            glowAlpha *= rangeFactor;
            if (glowAlpha < 0.001) continue;

            uint8_t* dst = buf.pixelAt(x, y);
            double da = fromByte(dst[3]);
            double outA = std::min(glowAlpha + da, 1.0);
            double blend = glowAlpha / std::max(outA, 0.001);
            dst[0] = toByte(clamp01(fromByte(dst[0]) * (1.0 - blend) + gr * blend));
            dst[1] = toByte(clamp01(fromByte(dst[1]) * (1.0 - blend) + gg * blend));
            dst[2] = toByte(clamp01(fromByte(dst[2]) * (1.0 - blend) + gb * blend));
            dst[3] = toByte(clamp01(outA));
        }
    }
}

void LayerStyle::renderInnerGlow(PixelBuffer& buf, int w, int h) const {
    std::vector<double> alpha = buildAlphaMask(buf, w, h);
    std::vector<double> inverted(w * h);
    for (int i = 0; i < w * h; ++i) {
        inverted[i] = 1.0 - alpha[i];
    }

    std::vector<double> blurred(w * h);
    gaussianBlur(inverted, blurred, w, h, m_innerGlow.size);

    double opacityFactor = m_innerGlow.opacity / 100.0;
    double gr = m_innerGlow.color.r;
    double gg = m_innerGlow.color.g;
    double gb = m_innerGlow.color.b;
    double rangeFactor = m_innerGlow.range / 100.0;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double srcAlpha = alpha[y * w + x];
            if (srcAlpha < 0.01) continue;

            int idx = y * w + x;
            double glowAlpha = std::max(0.0, blurred[idx] * (1.0 - srcAlpha)) * opacityFactor;
            glowAlpha *= rangeFactor;
            if (glowAlpha < 0.001) continue;

            uint8_t* dst = buf.pixelAt(x, y);
            double da = fromByte(dst[3]);
            double sa = clamp01(glowAlpha);
            double outA = std::min(da + sa * (1.0 - da), 1.0);
            double blend = (sa * (1.0 - da)) / std::max(outA, 0.001);
            dst[0] = toByte(clamp01(fromByte(dst[0]) * (1.0 - blend) + gr * blend));
            dst[1] = toByte(clamp01(fromByte(dst[1]) * (1.0 - blend) + gg * blend));
            dst[2] = toByte(clamp01(fromByte(dst[2]) * (1.0 - blend) + gb * blend));
            dst[3] = toByte(clamp01(outA));
        }
    }
}

void LayerStyle::renderBevelEmboss(PixelBuffer& buf, int w, int h) const {
    std::vector<double> alpha = buildAlphaMask(buf, w, h);
    int bevelSize = static_cast<int>(std::ceil(m_bevelEmboss.size));

    std::vector<double> normalsX(w * h, 0.0);
    std::vector<double> normalsY(w * h, 0.0);
    std::vector<double> normalsZ(w * h, 0.0);

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int idx = y * w + x;
            double left  = alpha[y * w + (x - 1)];
            double right = alpha[y * w + (x + 1)];
            double up    = alpha[(y - 1) * w + x];
            double down  = alpha[(y + 1) * w + x];
            double dx = right - left;
            double dy = down - up;
            double dz = 1.0 / std::max(m_bevelEmboss.depth * 0.01, 0.001);
            double len = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (len > 0.0001) {
                normalsX[idx] = dx / len;
                normalsY[idx] = dy / len;
                normalsZ[idx] = dz / len;
            } else {
                normalsX[idx] = 0.0;
                normalsY[idx] = 0.0;
                normalsZ[idx] = 1.0;
            }
        }
    }

    std::vector<double> normalMag(w * h);
    for (int i = 0; i < w * h; ++i) {
        normalMag[i] = std::sqrt(normalsX[i] * normalsX[i] + normalsY[i] * normalsY[i] + normalsZ[i] * normalsZ[i]);
    }

    if (bevelSize > 1) {
        gaussianBlur(normalsX, normalsX, w, h, bevelSize * 0.5);
        gaussianBlur(normalsY, normalsY, w, h, bevelSize * 0.5);
    }

    double lightAngleRad = m_bevelEmboss.angle * M_PI / 180.0;
    double altRad = m_bevelEmboss.altitude * M_PI / 180.0;
    Vec3 lightDir(
        std::cos(lightAngleRad) * std::cos(altRad),
        std::sin(lightAngleRad) * std::cos(altRad),
        std::sin(altRad)
    );

    double hlR = m_bevelEmboss.highlightColor.r;
    double hlG = m_bevelEmboss.highlightColor.g;
    double hlB = m_bevelEmboss.highlightColor.b;
    double hlA = m_bevelEmboss.highlightColor.a;

    double shR = m_bevelEmboss.shadowColor.r;
    double shG = m_bevelEmboss.shadowColor.g;
    double shB = m_bevelEmboss.shadowColor.b;
    double shA = m_bevelEmboss.shadowColor.a;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            double srcAlpha = alpha[idx];
            if (srcAlpha < 0.01) continue;

            double nx = normalsX[idx];
            double ny = normalsY[idx];
            double nz = normalsZ[idx];
            double len = std::sqrt(nx * nx + ny * ny + nz * nz);
            if (len < 0.0001) continue;
            nx /= len; ny /= len; nz /= len;

            double diffuse = std::max(0.0, nx * lightDir.x + ny * lightDir.y + nz * lightDir.z);

            Vec3 halfVec(lightDir.x, lightDir.y, lightDir.z + 1.0);
            double hLen = halfVec.length();
            if (hLen > 0.0001) {
                halfVec.x /= hLen;
                halfVec.y /= hLen;
                halfVec.z /= hLen;
            }
            double spec = std::max(0.0, nx * halfVec.x + ny * halfVec.y + nz * halfVec.z);
            spec = std::pow(spec, 20.0) * 0.5;

            bool isHighlight = diffuse > 0.0;
            double intensity = isHighlight ? diffuse : -diffuse;

            uint8_t* dst = buf.pixelAt(x, y);
            double da = fromByte(dst[3]);

            if (isHighlight) {
                double outA = clamp01(intensity * hlA * srcAlpha);
                double blend = outA * (1.0 - da);
                dst[0] = toByte(clamp01(fromByte(dst[0]) * (1.0 - blend) + hlR * blend + spec));
                dst[1] = toByte(clamp01(fromByte(dst[1]) * (1.0 - blend) + hlG * blend + spec));
                dst[2] = toByte(clamp01(fromByte(dst[2]) * (1.0 - blend) + hlB * blend + spec));
                dst[3] = toByte(clamp01(std::min(da + outA * (1.0 - da), 1.0)));
            } else {
                double outA = clamp01(intensity * shA * srcAlpha);
                double blend = outA * (1.0 - da);
                dst[0] = toByte(clamp01(fromByte(dst[0]) * (1.0 - blend) + shR * blend));
                dst[1] = toByte(clamp01(fromByte(dst[1]) * (1.0 - blend) + shG * blend));
                dst[2] = toByte(clamp01(fromByte(dst[2]) * (1.0 - blend) + shB * blend));
                dst[3] = toByte(clamp01(std::min(da + outA * (1.0 - da), 1.0)));
            }
        }
    }
}

void LayerStyle::renderStroke(PixelBuffer& buf, int w, int h) const {
    std::vector<double> alpha = buildAlphaMask(buf, w, h);
    int strokeRadius = static_cast<int>(std::ceil(m_stroke.size * 0.5));

    std::vector<double> strokeMask(w * h, 0.0);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (!isEdgePixel(alpha, w, h, x, y)) continue;

            for (int dy = -strokeRadius; dy <= strokeRadius; ++dy) {
                for (int dx = -strokeRadius; dx <= strokeRadius; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
                    double dist = std::sqrt(static_cast<double>(dx * dx + dy * dy));
                    if (dist <= strokeRadius) {
                        double edgeDist = strokeRadius - dist;
                        strokeMask[ny * w + nx] = std::max(strokeMask[ny * w + nx], 1.0);
                    }
                }
            }
        }
    }

    if (m_stroke.position == 1) { // inside
        for (int i = 0; i < w * h; ++i) {
            if (alpha[i] < 0.01) strokeMask[i] = 0.0;
        }
    } else if (m_stroke.position == 0) { // outside
        for (int i = 0; i < w * h; ++i) {
            if (alpha[i] > 0.01) strokeMask[i] = 0.0;
        }
    }

    double opacityFactor = m_stroke.opacity / 100.0;
    double sr = m_stroke.color.r;
    double sg = m_stroke.color.g;
    double sb = m_stroke.color.b;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            double sm = strokeMask[idx] * opacityFactor;
            if (sm < 0.001) continue;

            uint8_t* dst = buf.pixelAt(x, y);
            double da = fromByte(dst[3]);
            double sa = clamp01(sm);
            double outA = std::min(sa + da * (1.0 - sa), 1.0);
            double blend = sa / std::max(outA, 0.001);
            dst[0] = toByte(clamp01(fromByte(dst[0]) * (1.0 - blend) + sr * blend));
            dst[1] = toByte(clamp01(fromByte(dst[1]) * (1.0 - blend) + sg * blend));
            dst[2] = toByte(clamp01(fromByte(dst[2]) * (1.0 - blend) + sb * blend));
            dst[3] = toByte(clamp01(outA));
        }
    }
}

void LayerStyle::renderSatin(PixelBuffer& buf, int w, int h) const {
    std::vector<double> alpha = buildAlphaMask(buf, w, h);

    std::vector<double> blurred(w * h);
    gaussianBlur(alpha, blurred, w, h, m_satin.size);

    double rad = m_satin.angle * M_PI / 180.0;
    int offX = static_cast<int>(std::round(std::cos(rad) * m_satin.distance));
    int offY = static_cast<int>(std::round(std::sin(rad) * m_satin.distance));

    std::vector<double> offsetMask(w * h, 0.0);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int sx = std::clamp(x - offX, 0, w - 1);
            int sy = std::clamp(y - offY, 0, h - 1);
            offsetMask[y * w + x] = blurred[sy * w + sx];
        }
    }

    double opacityFactor = m_satin.opacity / 100.0;
    double sr = m_satin.color.r;
    double sg = m_satin.color.g;
    double sb = m_satin.color.b;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            double srcAlpha = alpha[idx];
            if (srcAlpha < 0.01) continue;

            double satinVal = std::abs(blurred[idx] - offsetMask[idx]);
            if (m_satin.invert) satinVal = 1.0 - satinVal;
            double satA = satinVal * opacityFactor * srcAlpha;
            if (satA < 0.001) continue;

            uint8_t* dst = buf.pixelAt(x, y);
            double da = fromByte(dst[3]);
            double sa = clamp01(satA);
            double outA = std::min(da + sa * (1.0 - da), 1.0);
            double blend = (sa * (1.0 - da)) / std::max(outA, 0.001);
            dst[0] = toByte(clamp01(fromByte(dst[0]) * (1.0 - blend) + sr * blend));
            dst[1] = toByte(clamp01(fromByte(dst[1]) * (1.0 - blend) + sg * blend));
            dst[2] = toByte(clamp01(fromByte(dst[2]) * (1.0 - blend) + sb * blend));
            dst[3] = toByte(clamp01(outA));
        }
    }
}

void LayerStyle::renderColorOverlay(PixelBuffer& buf, int w, int h) const {
    std::vector<double> alpha = buildAlphaMask(buf, w, h);
    double opacityFactor = m_colorOverlay.opacity / 100.0;
    double cr = m_colorOverlay.color.r;
    double cg = m_colorOverlay.color.g;
    double cb = m_colorOverlay.color.b;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            double srcAlpha = alpha[idx];
            if (srcAlpha < 0.01) continue;

            uint8_t* dst = buf.pixelAt(x, y);
            double da = fromByte(dst[3]);
            double sa = srcAlpha * opacityFactor;
            double outA = std::min(sa + da * (1.0 - sa), 1.0);
            double blend = (sa * (1.0 - da)) / std::max(outA, 0.001) + (1.0 - (1.0 - sa) * (1.0 - da)) * (1.0 - (1.0 - da)) / std::max(outA, 0.001);
            blend = std::clamp(blend, 0.0, 1.0);
            double finalBlend = sa * da + sa * (1.0 - da);
            finalBlend = std::clamp(finalBlend, 0.0, 1.0);
            dst[0] = toByte(clamp01(fromByte(dst[0]) * (1.0 - sa) + cr * sa));
            dst[1] = toByte(clamp01(fromByte(dst[1]) * (1.0 - sa) + cg * sa));
            dst[2] = toByte(clamp01(fromByte(dst[2]) * (1.0 - sa) + cb * sa));
            dst[3] = toByte(clamp01(std::min(outA, 1.0)));
        }
    }
}

void LayerStyle::renderGradientOverlay(PixelBuffer& buf, int w, int h) const {
    std::vector<double> alpha = buildAlphaMask(buf, w, h);
    double opacityFactor = m_gradientOverlay.opacity / 100.0;

    double rad = m_gradientOverlay.angle * M_PI / 180.0;
    double cosA = std::cos(rad);
    double sinA = std::sin(rad);
    double scale = m_gradientOverlay.scale / 100.0;

    double c1r = m_gradientOverlay.color1.r;
    double c1g = m_gradientOverlay.color1.g;
    double c1b = m_gradientOverlay.color1.b;
    double c2r = m_gradientOverlay.color2.r;
    double c2g = m_gradientOverlay.color2.g;
    double c2b = m_gradientOverlay.color2.b;
    if (m_gradientOverlay.reverse) {
        std::swap(c1r, c2r);
        std::swap(c1g, c2g);
        std::swap(c1b, c2b);
    }

    double cx = w * 0.5;
    double cy = h * 0.5;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            double srcAlpha = alpha[idx];
            if (srcAlpha < 0.01) continue;

            double t;
            if (m_gradientOverlay.gradientType == 0) { // linear
                double dx = (x - cx) / (w * 0.5);
                double dy = (y - cy) / (h * 0.5);
                t = (dx * cosA + dy * sinA) * scale + 0.5;
            } else { // radial
                double dx = (x - cx) / (w * 0.5);
                double dy = (y - cy) / (h * 0.5);
                t = std::sqrt(dx * dx + dy * dy) * scale;
            }
            t = clamp01(t);

            double gr = c1r + (c2r - c1r) * t;
            double gg = c1g + (c2g - c1g) * t;
            double gb = c1b + (c2b - c1b) * t;

            uint8_t* dst = buf.pixelAt(x, y);
            double da = fromByte(dst[3]);
            double sa = srcAlpha * opacityFactor;
            double outA = std::min(sa + da * (1.0 - sa), 1.0);
            dst[0] = toByte(clamp01(fromByte(dst[0]) * (1.0 - sa) + gr * sa));
            dst[1] = toByte(clamp01(fromByte(dst[1]) * (1.0 - sa) + gg * sa));
            dst[2] = toByte(clamp01(fromByte(dst[2]) * (1.0 - sa) + gb * sa));
            dst[3] = toByte(clamp01(outA));
        }
    }
}

} // namespace FreeEffect
