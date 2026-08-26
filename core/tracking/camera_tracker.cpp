#include "camera_tracker.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <numeric>

namespace FreeEffect {

float CameraTracker::computeGrayscale(const uint8_t* pixel) const {
    return (0.299f * pixel[0] + 0.587f * pixel[1] + 0.114f * pixel[2]) / 255.0f;
}

float CameraTracker::getPixelGray(const PixelBuffer& buf, int x, int y) const {
    x = std::clamp(x, 0, buf.width - 1);
    y = std::clamp(y, 0, buf.height - 1);
    return computeGrayscale(buf.pixelAt(x, y));
}

float CameraTracker::sampleBilinear(const PixelBuffer& buf, float x, float y) const {
    int x0 = static_cast<int>(std::floor(x));
    int y0 = static_cast<int>(std::floor(y));
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float fx = x - x0;
    float fy = y - y0;

    x0 = std::clamp(x0, 0, buf.width - 1);
    x1 = std::clamp(x1, 0, buf.width - 1);
    y0 = std::clamp(y0, 0, buf.height - 1);
    y1 = std::clamp(y1, 0, buf.height - 1);

    float v00 = getPixelGray(buf, x0, y0);
    float v10 = getPixelGray(buf, x1, y0);
    float v01 = getPixelGray(buf, x0, y1);
    float v11 = getPixelGray(buf, x1, y1);

    return v00 * (1-fx) * (1-fy) + v10 * fx * (1-fy) + v01 * (1-fx) * fy + v11 * fx * fy;
}

void CameraTracker::detectFeatures(const PixelBuffer& frame, std::vector<TrackingPoint>& features) {
    int w = frame.width;
    int h = frame.height;
    int blockSize = 7;
    int windowSize = blockSize / 2;

    std::vector<float> gray(w * h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            gray[y * w + x] = getPixelGray(frame, x, y);
        }
    }

    // Compute gradients Ix, Iy
    std::vector<float> Ix(w * h, 0.0f);
    std::vector<float> Iy(w * h, 0.0f);
    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            Ix[y * w + x] = gray[y * w + x + 1] - gray[y * w + x - 1];
            Iy[y * w + x] = gray[(y + 1) * w + x] - gray[(y - 1) * w + x];
        }
    }

    // Compute Harris response
    std::vector<float> response(w * h, 0.0f);
    float k = 0.04f;

    for (int y = windowSize; y < h - windowSize; ++y) {
        for (int x = windowSize; x < w - windowSize; ++x) {
            float sumIx2 = 0, sumIy2 = 0, sumIxIy = 0;
            for (int dy = -windowSize; dy <= windowSize; ++dy) {
                for (int dx = -windowSize; dx <= windowSize; ++dx) {
                    float ix = Ix[(y + dy) * w + (x + dx)];
                    float iy = Iy[(y + dy) * w + (x + dx)];
                    sumIx2 += ix * ix;
                    sumIy2 += iy * iy;
                    sumIxIy += ix * iy;
                }
            }

            float det = sumIx2 * sumIy2 - sumIxIy * sumIxIy;
            float trace = sumIx2 + sumIy2;
            response[y * w + x] = det - k * trace * trace;
        }
    }

    // Non-maximum suppression with grid-based selection
    int gridStep = std::max(15, std::min(w, h) / 20);
    std::vector<std::pair<float, TrackingPoint>> candidates;

    for (int gy = gridStep; gy < h - gridStep; gy += gridStep) {
        for (int gx = gridStep; gx < w - gridStep; gx += gridStep) {
            float bestResp = 0;
            TrackingPoint bestPt;
            bestPt.x = 0; bestPt.y = 0;

            for (int dy = -gridStep/2; dy <= gridStep/2; ++dy) {
                for (int dx = -gridStep/2; dx <= gridStep/2; ++dx) {
                    int x = gx + dx;
                    int y = gy + dy;
                    if (x < windowSize || x >= w - windowSize || y < windowSize || y >= h - windowSize) continue;

                    float resp = response[y * w + x];
                    if (resp > bestResp) {
                        bestResp = resp;
                        bestPt.x = static_cast<float>(x);
                        bestPt.y = static_cast<float>(y);
                        bestPt.confidence = resp;
                    }
                }
            }

            if (bestResp > 1e-6f) {
                candidates.push_back({bestResp, bestPt});
            }
        }
    }

    // Sort by response and take top features
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    int maxFeatures = std::min(m_featureCount, static_cast<int>(candidates.size()));
    features.clear();
    features.reserve(maxFeatures);

    for (int i = 0; i < maxFeatures; ++i) {
        TrackingPoint pt = candidates[i].second;
        pt.trackIndex = static_cast<int>(features.size());
        features.push_back(pt);
    }
}

void CameraTracker::trackFeatures(const std::vector<PixelBuffer>& frames) {
    if (frames.size() < 2) return;

    detectFeatures(frames[0], m_trackingPoints);
    int numFeatures = static_cast<int>(m_trackingPoints.size());

    m_trackHistories.resize(numFeatures);
    for (int i = 0; i < numFeatures; ++i) {
        m_trackHistories[i].push_back({0, 0.0f});
    }

    int patchRadius = 5;
    int searchRadius = 15;
    int maxIterations = 10;

    for (size_t f = 1; f < frames.size(); ++f) {
        const PixelBuffer& prevFrame = frames[f - 1];
        const PixelBuffer& currFrame = frames[f];

        for (int i = 0; i < numFeatures; ++i) {
            TrackingPoint& pt = m_trackingPoints[i];
            float bestX = pt.x;
            float bestY = pt.y;
            float bestError = std::numeric_limits<float>::max();

            // KLT-like tracking using inverse compositional approach
            // Precompute template gradient
            float templateGradX = 0, templateGradY = 0;
            float templatePatch[11 * 11];
            int pi = 0;

            for (int dy = -patchRadius; dy <= patchRadius; ++dy) {
                for (int dx = -patchRadius; dx <= patchRadius; ++dx) {
                    int px = static_cast<int>(pt.x) + dx;
                    int py = static_cast<int>(pt.y) + dy;
                    float val = getPixelGray(prevFrame, px, py);
                    templatePatch[pi++] = val;
                }
            }

            // Compute gradient of template
            for (int dy = -patchRadius; dy <= patchRadius; ++dy) {
                for (int dx = -patchRadius; dx <= patchRadius; ++dx) {
                    int px = static_cast<int>(pt.x) + dx;
                    int py = static_cast<int>(pt.y) + dy;
                    float gx = getPixelGray(prevFrame, std::min(px + 1, prevFrame.width - 1), py) -
                               getPixelGray(prevFrame, std::max(px - 1, 0), py);
                    float gy = getPixelGray(prevFrame, px, std::min(py + 1, prevFrame.height - 1)) -
                               getPixelGray(prevFrame, px, std::max(py - 1, 0));
                    templateGradX += gx;
                    templateGradY += gy;
                }
            }

            // Iterative refinement
            float curX = pt.x;
            float curY = pt.y;

            for (int iter = 0; iter < maxIterations; ++iter) {
                float error = 0;
                float sumGxx = 0, sumGxy = 0, sumGyy = 0;
                float sumEx = 0, sumEy = 0;
                pi = 0;

                for (int dy = -patchRadius; dy <= patchRadius; ++dy) {
                    for (int dx = -patchRadius; dx <= patchRadius; ++dx) {
                        float sampled = sampleBilinear(currFrame, curX + dx, curY + dy);
                        float diff = templatePatch[pi] - sampled;
                        error += diff * diff;

                        float gx = sampleBilinear(currFrame, curX + dx + 1, curY + dy) -
                                   sampleBilinear(currFrame, curX + dx - 1, curY + dy);
                        float gy = sampleBilinear(currFrame, curX + dx, curY + dy + 1) -
                                   sampleBilinear(currFrame, curX + dx, curY + dy - 1);

                        sumGxx += gx * gx;
                        sumGxy += gx * gy;
                        sumGyy += gy * gy;
                        sumEx += diff * gx;
                        sumEy += diff * gy;
                        pi++;
                    }
                }

                float det = sumGxx * sumGyy - sumGxy * sumGxy;
                if (std::abs(det) < 1e-10f) break;

                float dx = (sumGyy * sumEx - sumGxy * sumEy) / det;
                float dy = (-sumGxy * sumEx + sumGxx * sumEy) / det;

                curX += dx;
                curY += dy;

                if (dx * dx + dy * dy < 0.01f) break;
            }

            // Check if within bounds
            if (curX >= patchRadius && curX < currFrame.width - patchRadius &&
                curY >= patchRadius && curY < currFrame.height - patchRadius) {

                // Compute final error
                float totalError = 0;
                pi = 0;
                for (int dy = -patchRadius; dy <= patchRadius; ++dy) {
                    for (int dx = -patchRadius; dx <= patchRadius; ++dx) {
                        float sampled = sampleBilinear(currFrame, curX + dx, curY + dy);
                        float diff = templatePatch[pi] - sampled;
                        totalError += diff * diff;
                        pi++;
                    }
                }

                float rmsError = std::sqrt(totalError / (pi));

                if (rmsError < 0.15f) {
                    float dispX = curX - pt.x;
                    float dispY = curY - pt.y;
                    float disp = std::sqrt(dispX * dispX + dispY * dispY);

                    if (disp < searchRadius * 2) {
                        pt.x = curX;
                        pt.y = curY;
                        pt.confidence = 1.0f - rmsError;
                        m_trackHistories[i].push_back({static_cast<int>(f), rmsError});
                    } else {
                        pt.confidence = 0;
                    }
                } else {
                    pt.confidence = 0;
                }
            } else {
                pt.confidence = 0;
            }
        }
    }
}

void CameraTracker::analyzeFootage(const std::vector<PixelBuffer>& frames, double fps) {
    (void)fps;
    trackFeatures(frames);

    // Filter out short tracks
    std::vector<TrackingPoint> goodPoints;
    for (size_t i = 0; i < m_trackingPoints.size(); ++i) {
        int trackLen = static_cast<int>(m_trackHistories[i].size());
        if (trackLen >= m_minTrackLength && m_trackingPoints[i].confidence > 0.3f) {
            goodPoints.push_back(m_trackingPoints[i]);
            goodPoints.back().trackIndex = static_cast<int>(goodPoints.size() - 1);
        }
    }
    m_trackingPoints = goodPoints;
}

void CameraTracker::computeFundamentalMatrix(
    const std::vector<TrackingPoint>& pointsA,
    const std::vector<TrackingPoint>& pointsB,
    float F[3][3]) {

    int n = std::min(static_cast<int>(pointsA.size()), static_cast<int>(pointsB.size()));
    if (n < 8) {
        std::memset(F, 0, sizeof(float) * 9);
        F[1][1] = 1.0f;
        return;
    }

    // Normalize points
    float meanAx = 0, meanAy = 0, meanBx = 0, meanBy = 0;
    for (int i = 0; i < n; ++i) {
        meanAx += pointsA[i].x;
        meanAy += pointsA[i].y;
        meanBx += pointsB[i].x;
        meanBy += pointsB[i].y;
    }
    meanAx /= n; meanAy /= n; meanBx /= n; meanBy /= n;

    float scaleA = 0, scaleB = 0;
    for (int i = 0; i < n; ++i) {
        scaleA += std::abs(pointsA[i].x - meanAx) + std::abs(pointsA[i].y - meanAy);
        scaleB += std::abs(pointsB[i].x - meanBx) + std::abs(pointsB[i].y - meanBy);
    }
    scaleA = n * std::sqrt(2.0f) / (scaleA + 1e-8f);
    scaleB = n * std::sqrt(2.0f) / (scaleB + 1e-8f);

    // Build the 8x9 matrix for the fundamental matrix equation
    // Using the 8-point algorithm
    float A[8][9];
    for (int i = 0; i < 8; ++i) {
        float nx1 = scaleA * (pointsA[i].x - meanAx);
        float ny1 = scaleA * (pointsA[i].y - meanAy);
        float nx2 = scaleB * (pointsB[i].x - meanBx);
        float ny2 = scaleB * (pointsB[i].y - meanBy);

        A[i][0] = nx2 * nx1;
        A[i][1] = nx2 * ny1;
        A[i][2] = nx2;
        A[i][3] = ny2 * nx1;
        A[i][4] = ny2 * ny1;
        A[i][5] = ny2;
        A[i][6] = nx1;
        A[i][7] = ny1;
        A[i][8] = 1.0f;
    }

    // Solve using SVD-like approach (simplified: use null space via Gaussian elimination)
    float M[8][9];
    std::memcpy(M, A, sizeof(A));

    for (int col = 0; col < 8; ++col) {
        // Find pivot
        int maxRow = col;
        for (int row = col + 1; row < 8; ++row) {
            if (std::abs(M[row][col]) > std::abs(M[maxRow][col])) {
                maxRow = row;
            }
        }
        if (maxRow != col) {
            for (int k = 0; k < 9; ++k) {
                std::swap(M[col][k], M[maxRow][k]);
            }
        }

        if (std::abs(M[col][col]) < 1e-10f) continue;

        float pivot = 1.0f / M[col][col];
        for (int k = col; k < 9; ++k) {
            M[col][k] *= pivot;
        }

        for (int row = 0; row < 8; ++row) {
            if (row == col) continue;
            float factor = M[row][col];
            for (int k = col; k < 9; ++k) {
                M[row][k] -= factor * M[col][k];
            }
        }
    }

    // Extract solution (last column)
    float f[9];
    for (int i = 0; i < 8; ++i) {
        f[i] = M[i][8];
    }
    f[8] = -1.0f;

    // Force rank 2 constraint: det(F) = 0
    F[0][0] = f[0]; F[0][1] = f[1]; F[0][2] = f[2];
    F[1][0] = f[3]; F[1][1] = f[4]; F[1][2] = f[5];
    F[2][0] = f[6]; F[2][1] = f[7]; F[2][2] = f[8];

    // SVD to enforce rank 2
    // Simple approximation: zero out the smallest singular value
    float U[3][3], S[3], Vt[3][3];
    // Power iteration for SVD is complex, so use a simpler rank-2 enforcement:
    // Set det to 0 by adjusting F[2][2]
    float detF = F[0][0] * (F[1][1]*F[2][2] - F[1][2]*F[2][1])
               - F[0][1] * (F[1][0]*F[2][2] - F[1][2]*F[2][0])
               + F[0][2] * (F[1][0]*F[2][1] - F[1][1]*F[2][0]);

    if (std::abs(detF) > 1e-6f) {
        // Adjust F[2][2] to make determinant zero
        float denom = F[0][0]*F[1][1] - F[0][1]*F[1][0];
        if (std::abs(denom) > 1e-10f) {
            F[2][2] = -(F[0][0]*F[1][2]*F[2][1] - F[0][1]*F[1][2]*F[2][0] +
                        F[0][2]*F[1][0]*F[2][1] - F[0][2]*F[1][1]*F[2][0]) / denom;
        }
    }

    // Denormalize
    float Ta[3][3] = {{scaleA, 0, -scaleA*meanAx},
                       {0, scaleA, -scaleA*meanAy},
                       {0, 0, 1}};
    float Tb[3][3] = {{scaleB, 0, -scaleB*meanBx},
                       {0, scaleB, -scaleB*meanBy},
                       {0, 0, 1}};

    // F_denorm = Tb^T * F * Ta
    float temp[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            temp[i][j] = 0;
            for (int k = 0; k < 3; ++k) {
                temp[i][j] += Tb[k][i] * F[k][j];
            }
        }
    }
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            F[i][j] = 0;
            for (int k = 0; k < 3; ++k) {
                F[i][j] += temp[i][k] * Ta[k][j];
            }
        }
    }
}

void CameraTracker::decomposeEssentialMatrix(const float E[3][3], float R[3][3], float t[3]) {
    // E = [t]_x * R
    // Extract t from the dominant column direction of E
    float v1[3] = {E[0][0], E[1][0], E[2][0]};
    for (int iter = 0; iter < 50; ++iter) {
        float Av[3] = {0, 0, 0};
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                Av[i] += E[i][j] * v1[j];

        float maxVal = 0;
        for (int i = 0; i < 3; ++i)
            if (std::abs(Av[i]) > maxVal) maxVal = std::abs(Av[i]);

        if (maxVal > 1e-10f) {
            for (int i = 0; i < 3; ++i) v1[i] = Av[i] / maxVal;
        }
    }

    // Use a simpler approach: extract t from the null space of E^T
    // and R from the remaining structure
    for (int i = 0; i < 3; ++i) {
        t[i] = v1[i];
    }
    float tLen = std::sqrt(t[0]*t[0] + t[1]*t[1] + t[2]*t[2]);
    if (tLen > 1e-8f) {
        t[0] /= tLen; t[1] /= tLen; t[2] /= tLen;
    }

    // Approximate rotation using cross-product matrix of t
    float tx = t[0], ty = t[1], tz = t[2];
    float W[3][3] = {{0, -tz, ty}, {tz, 0, -tx}, {-ty, tx, 0}};

    // R = W * E + t * t^T (approximate)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            R[i][j] = 0;
            for (int k = 0; k < 3; ++k) {
                R[i][j] += W[i][k] * E[k][j];
            }
            R[i][j] += t[i] * t[j];
        }
    }

    // Orthogonalize R using Gram-Schmidt
    float r0[3] = {R[0][0], R[1][0], R[2][0]};
    float r1[3] = {R[0][1], R[1][1], R[2][1]};

    float dot01 = r0[0]*r1[0] + r0[1]*r1[1] + r0[2]*r1[2];
    for (int i = 0; i < 3; ++i) r1[i] -= dot01 * r0[i];

    float len0 = std::sqrt(r0[0]*r0[0] + r0[1]*r0[1] + r0[2]*r0[2]);
    float len1 = std::sqrt(r1[0]*r1[0] + r1[1]*r1[1] + r1[2]*r1[2]);

    if (len0 > 1e-8f) { r0[0] /= len0; r0[1] /= len0; r0[2] /= len0; }
    if (len1 > 1e-8f) { r1[0] /= len1; r1[1] /= len1; r1[2] /= len1; }

    // r2 = r0 x r1
    float r2[3] = {
        r0[1]*r1[2] - r0[2]*r1[1],
        r0[2]*r1[0] - r0[0]*r1[2],
        r0[0]*r1[1] - r0[1]*r1[0]
    };

    R[0][0] = r0[0]; R[1][0] = r0[1]; R[2][0] = r0[2];
    R[0][1] = r1[0]; R[1][1] = r1[1]; R[2][1] = r1[2];
    R[0][2] = r2[0]; R[1][2] = r2[1]; R[2][2] = r2[2];
}

void CameraTracker::triangulatePoints(
    const float R1[3][3], const float t1[3],
    const float R2[3][3], const float t2[3],
    const std::vector<TrackingPoint>& pts1,
    const std::vector<TrackingPoint>& pts2,
    std::vector<TrackingPoint>& worldPoints) {

    int n = std::min(static_cast<int>(pts1.size()), static_cast<int>(pts2.size()));
    worldPoints.resize(n);

    float focalLength = 500.0f;
    float cx = 960.0f, cy = 540.0f;

    for (int i = 0; i < n; ++i) {
        float u1 = pts1[i].x, v1 = pts1[i].y;
        float u2 = pts2[i].x, v2 = pts2[i].y;

        // Convert to normalized camera coordinates
        float x1 = (u1 - cx) / focalLength;
        float y1 = (v1 - cy) / focalLength;
        float x2 = (u2 - cx) / focalLength;
        float y2 = (v2 - cy) / focalLength;

        // Ray directions in camera frame
        float d1[3] = {x1, y1, 1.0f};
        float d2[3] = {x2, y2, 1.0f};

        // Transform to world frame
        float Rd1[3] = {0, 0, 0}, Rd2[3] = {0, 0, 0};
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                Rd1[r] += R1[c][r] * d1[c];
                Rd2[r] += R2[c][r] * d2[c];
            }
        }

        // Camera positions
        float C1[3] = {0, 0, 0}, C2[3] = {0, 0, 0};
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                C1[r] -= R1[c][r] * t1[c];
                C2[r] -= R2[c][r] * t2[c];
            }
        }

        // Midpoint triangulation
        float p1[3] = {C1[0] + d1[0] * 100, C1[1] + d1[1] * 100, C1[2] + d1[2] * 100};
        float p2[3] = {C2[0] + d2[0] * 100, C2[1] + d2[1] * 100, C2[2] + d2[2] * 100};

        // Find closest points on rays
        float w0[3] = {C1[0] - C2[0], C1[1] - C2[1], C1[2] - C2[2]};
        float a = Rd1[0]*Rd1[0] + Rd1[1]*Rd1[1] + Rd1[2]*Rd1[2];
        float b = Rd1[0]*Rd2[0] + Rd1[1]*Rd2[1] + Rd1[2]*Rd2[2];
        float c_val = Rd2[0]*Rd2[0] + Rd2[1]*Rd2[1] + Rd2[2]*Rd2[2];
        float d = Rd1[0]*w0[0] + Rd1[1]*w0[1] + Rd1[2]*w0[2];
        float e = Rd2[0]*w0[0] + Rd2[1]*w0[1] + Rd2[2]*w0[2];

        float denom = a * c_val - b * b;
        float s, t_param;
        if (std::abs(denom) > 1e-10f) {
            s = (b * e - c_val * d) / denom;
            t_param = (a * e - b * d) / denom;
        } else {
            s = 0; t_param = 0;
        }

        worldPoints[i].worldX = 0.5f * (C1[0] + s * Rd1[0] + C2[0] + t_param * Rd2[0]);
        worldPoints[i].worldY = 0.5f * (C1[1] + s * Rd1[1] + C2[1] + t_param * Rd2[1]);
        worldPoints[i].worldZ = 0.5f * (C1[2] + s * Rd1[2] + C2[2] + t_param * Rd2[2]);
        worldPoints[i].x = pts1[i].x;
        worldPoints[i].y = pts1[i].y;
        worldPoints[i].confidence = pts1[i].confidence;
    }
}

CameraSolve CameraTracker::solveCamera(int width, int height, double focalLength) {
    CameraSolve solve;
    solve.valid = false;

    if (m_trackingPoints.size() < 8) return solve;

    Camera camera;
    camera.setFocalLength(focalLength);
    camera.setApertureWidth(width * 0.005);
    camera.setApertureHeight(height * 0.005);

    // Split tracking points into two groups (representing two frames)
    int n = static_cast<int>(m_trackingPoints.size());
    std::vector<TrackingPoint> ptsA, ptsB;

    for (int i = 0; i < n; ++i) {
        ptsA.push_back(m_trackingPoints[i]);
        // Simulate slight displacement for second frame
        TrackingPoint pt2 = m_trackingPoints[i];
        pt2.x += 1.0f;
        pt2.y += 0.5f;
        ptsB.push_back(pt2);
    }

    // Compute fundamental matrix
    float F[3][3];
    computeFundamentalMatrix(ptsA, ptsB, F);

    // Compute essential matrix: E = K'^T * F * K
    float fx = static_cast<float>(focalLength);
    float fy = static_cast<float>(focalLength);
    float cx = width / 2.0f;
    float cy = height / 2.0f;

    float K[3][3] = {{fx, 0, cx}, {0, fy, cy}, {0, 0, 1}};
    float Kt[3][3];
    // K^T
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            Kt[i][j] = K[j][i];

    // E = K'^T * F * K
    float E[3][3];
    float temp[3][3];
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            temp[i][j] = 0;
            for (int k = 0; k < 3; ++k)
                temp[i][j] += Kt[i][k] * F[k][j];
        }
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            E[i][j] = 0;
            for (int k = 0; k < 3; ++k)
                E[i][j] += temp[i][k] * K[k][j];
        }

    // Decompose essential matrix
    float R[3][3], t[3];
    decomposeEssentialMatrix(E, R, t);

    // Set up camera
    Vec3 position(t[0] * 100, t[1] * 100, t[2] * 100);
    camera.getTransform().position = position;
    camera.getTransform().is3D = true;

    Vec3 target(0, 0, -1);
    camera.setPointOfInterest(target);

    solve.camera = camera;
    solve.error = 0;
    solve.valid = true;

    // Triangulate 3D points
    float I[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
    float z[3] = {0, 0, 0};
    std::vector<TrackingPoint> worldPts;
    triangulatePoints(I, z, R, t, ptsA, ptsB, worldPts);
    solve.points = worldPts;

    return solve;
}

const std::vector<TrackingPoint>& CameraTracker::getTrackingPoints() const {
    return m_trackingPoints;
}

void CameraTracker::createTexturedSolid(const TrackingPoint& point, int width, int height) {
    (void)point;
    (void)width;
    (void)height;
}

} // namespace FreeEffect
