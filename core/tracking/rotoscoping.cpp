#include "rotoscoping.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <set>

namespace FreeEffect {

void RotoBrush::addStroke(double time, const RotoStroke& stroke) {
    RotoStroke s = stroke;
    s.time = time;

    auto it = m_strokes.begin();
    while (it != m_strokes.end() && it->time < time) ++it;
    m_strokes.insert(it, s);

    RotoFrame frame;
    frame.time = time;
    frame.contour = computeContour(s, 1920, 1080);
    frame.propagated = false;
    m_frames[time] = frame;
}

bool RotoBrush::pointInPolygon(double px, double py, const std::vector<Vec2>& polygon) const {
    if (polygon.size() < 3) return false;

    bool inside = false;
    size_t n = polygon.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        double xi = polygon[i].x, yi = polygon[i].y;
        double xj = polygon[j].x, yj = polygon[j].y;

        if (((yi > py) != (yj > py)) &&
            (px < (xj - xi) * (py - yi) / (yj - yi) + xi)) {
            inside = !inside;
        }
    }
    return inside;
}

std::vector<MaskVertex> RotoBrush::computeContour(const RotoStroke& stroke, int width, int height) const {
    int gridW = width / 4;
    int gridH = height / 4;

    std::vector<std::vector<bool>> fgMask(gridW, std::vector<bool>(gridH, false));
    std::vector<std::vector<bool>> bgMask(gridW, std::vector<bool>(gridH, false));

    auto markRegion = [&](const std::vector<Vec2>& points, std::vector<std::vector<bool>>& mask, bool mark) {
        for (const auto& pt : points) {
            int gx = static_cast<int>(pt.x / 4.0);
            int gy = static_cast<int>(pt.y / 4.0);
            int r = m_brushRadius / 4;
            for (int dy = -r; dy <= r; ++dy) {
                for (int dx = -r; dx <= r; ++dx) {
                    int nx = gx + dx;
                    int ny = gy + dy;
                    if (nx >= 0 && nx < gridW && ny >= 0 && ny < gridH) {
                        if (dx * dx + dy * dy <= r * r) {
                            mask[nx][ny] = mark;
                        }
                    }
                }
            }
        }
    };

    markRegion(stroke.foregroundPoints, fgMask, true);
    markRegion(stroke.backgroundPoints, bgMask, true);

    std::vector<std::vector<bool>> combined(gridW, std::vector<bool>(gridH, false));
    for (int x = 0; x < gridW; ++x) {
        for (int y = 0; y < gridH; ++y) {
            combined[x][y] = fgMask[x][y] && !bgMask[x][y];
        }
    }

    std::vector<MaskVertex> contour;
    traceContour(combined, gridW, gridH, contour);

    for (auto& v : contour) {
        v.position.x *= 4.0;
        v.position.y *= 4.0;
    }

    return contour;
}

void RotoBrush::traceContour(const std::vector<std::vector<bool>>& mask, int w, int h,
                              std::vector<MaskVertex>& contour) const {
    const int dx[] = {1, 0, -1, 0};
    const int dy[] = {0, 1, 0, -1};

    int startX = -1, startY = -1;
    for (int x = 0; x < w && startX < 0; ++x) {
        for (int y = 0; y < h; ++y) {
            if (mask[x][y]) {
                bool onEdge = false;
                for (int d = 0; d < 4; ++d) {
                    int nx = x + dx[d];
                    int ny = y + dy[d];
                    if (nx < 0 || nx >= w || ny < 0 || ny >= h || !mask[nx][ny]) {
                        onEdge = true;
                        break;
                    }
                }
                if (onEdge) {
                    startX = x;
                    startY = y;
                    break;
                }
            }
        }
    }

    if (startX < 0) return;

    std::set<std::pair<int,int>> visited;
    int cx = startX, cy = startY;

    do {
        MaskVertex v;
        v.position.x = static_cast<double>(cx);
        v.position.y = static_cast<double>(cy);
        v.inHandle = {0, 0, 0, 0};
        v.outHandle = {0, 0, 0, 0};
        contour.push_back(v);
        visited.insert({cx, cy});

        bool found = false;
        for (int d = 0; d < 4; ++d) {
            int nx = cx + dx[d];
            int ny = cy + dy[d];
            if (nx >= 0 && nx < w && ny >= 0 && ny < h &&
                mask[nx][ny] && visited.find({nx, ny}) == visited.end()) {
                bool onEdge = false;
                for (int dd = 0; dd < 4; ++dd) {
                    int nnx = nx + dx[dd];
                    int nny = ny + dy[dd];
                    if (nnx < 0 || nnx >= w || nny < 0 || nny >= h || !mask[nnx][nny]) {
                        onEdge = true;
                        break;
                    }
                }
                if (onEdge) {
                    cx = nx;
                    cy = ny;
                    found = true;
                    break;
                }
            }
        }

        if (!found) break;

    } while (cx != startX || cy != startY);
}

RotoFrame RotoBrush::propagateFrame(const RotoFrame& ref, const RotoStroke& stroke) const {
    RotoFrame result;
    result.time = ref.time + (1.0 / 30.0);
    result.propagated = true;

    if (ref.contour.empty()) {
        result.contour = ref.contour;
        return result;
    }

    double meanDx = 0, meanDy = 0;
    int fgCount = static_cast<int>(stroke.foregroundPoints.size());
    if (fgCount > 1) {
        for (int i = 1; i < fgCount; ++i) {
            meanDx += stroke.foregroundPoints[i].x - stroke.foregroundPoints[i - 1].x;
            meanDy += stroke.foregroundPoints[i].y - stroke.foregroundPoints[i - 1].y;
        }
        meanDx /= (fgCount - 1);
        meanDy /= (fgCount - 1);
    }

    double scale = 1.0;
    double maxDist = 0;
    for (const auto& v : ref.contour) {
        double dist = std::sqrt(v.position.x * v.position.x + v.position.y * v.position.y);
        maxDist = std::max(maxDist, dist);
    }

    result.contour.resize(ref.contour.size());
    for (size_t i = 0; i < ref.contour.size(); ++i) {
        double x = ref.contour[i].position.x + meanDx;
        double y = ref.contour[i].position.y + meanDy;
        result.contour[i].position.x = x;
        result.contour[i].position.y = y;
        result.contour[i].inHandle = ref.contour[i].inHandle;
        result.contour[i].outHandle = ref.contour[i].outHandle;
    }

    return result;
}

void RotoBrush::propagateForward(double startTime, int frameCount) {
    auto it = m_frames.find(startTime);
    if (it == m_frames.end()) return;

    RotoFrame current = it->second;
    const RotoStroke* activeStroke = nullptr;
    for (const auto& s : m_strokes) {
        if (s.time <= startTime) activeStroke = &s;
    }
    if (!activeStroke) return;

    double frameDur = 1.0 / 30.0;
    for (int i = 0; i < frameCount; ++i) {
        RotoFrame next = propagateFrame(current, *activeStroke);
        next.time = startTime + (i + 1) * frameDur;
        m_frames[next.time] = next;
        current = next;
    }
}

void RotoBrush::propagateBackward(double startTime, int frameCount) {
    auto it = m_frames.find(startTime);
    if (it == m_frames.end()) return;

    RotoFrame current = it->second;
    const RotoStroke* activeStroke = nullptr;
    for (const auto& s : m_strokes) {
        if (s.time <= startTime) activeStroke = &s;
    }
    if (!activeStroke) return;

    double frameDur = 1.0 / 30.0;
    for (int i = 0; i < frameCount; ++i) {
        RotoFrame prev;
        prev.time = startTime - (i + 1) * frameDur;
        prev.propagated = true;

        double meanDx = 0, meanDy = 0;
        int fgCount = static_cast<int>(activeStroke->foregroundPoints.size());
        if (fgCount > 1) {
            for (int j = 1; j < fgCount; ++j) {
                meanDx += activeStroke->foregroundPoints[j].x - activeStroke->foregroundPoints[j - 1].x;
                meanDy += activeStroke->foregroundPoints[j].y - activeStroke->foregroundPoints[j - 1].y;
            }
            meanDx /= (fgCount - 1);
            meanDy /= (fgCount - 1);
        }

        prev.contour.resize(current.contour.size());
        for (size_t j = 0; j < current.contour.size(); ++j) {
            prev.contour[j].position.x = current.contour[j].position.x - meanDx;
            prev.contour[j].position.y = current.contour[j].position.y - meanDy;
            prev.contour[j].inHandle = current.contour[j].inHandle;
            prev.contour[j].outHandle = current.contour[j].outHandle;
        }

        m_frames[prev.time] = prev;
        current = prev;
    }
}

std::vector<MaskVertex> RotoBrush::getContour(double time) const {
    auto it = m_frames.lower_bound(time);
    if (it == m_frames.end() && !m_frames.empty()) {
        return m_frames.rbegin()->second.contour;
    }
    if (it == m_frames.begin()) {
        return it->second.contour;
    }

    auto prev = std::prev(it);
    double t0 = prev->first;
    double t1 = it->first;
    double t = (t1 > t0) ? (time - t0) / (t1 - t0) : 0.0;
    t = std::clamp(t, 0.0, 1.0);

    const auto& c0 = prev->second.contour;
    const auto& c1 = it->second.contour;
    size_t count = std::max(c0.size(), c1.size());

    std::vector<MaskVertex> result(count);
    for (size_t i = 0; i < count; ++i) {
        const auto& v0 = c0[i % c0.size()];
        const auto& v1 = c1[i % c1.size()];
        result[i].position.x = v0.position.x + t * (v1.position.x - v0.position.x);
        result[i].position.y = v0.position.y + t * (v1.position.y - v0.position.y);
        result[i].inHandle = v0.inHandle;
        result[i].outHandle = v0.outHandle;
    }

    return result;
}

PixelBuffer RotoBrush::generateMatte(int width, int height, double time) const {
    PixelBuffer matte;
    matte.resize(width, height);

    auto contour = getContour(time);
    if (contour.size() < 3) return matte;

    std::vector<Vec2> poly(contour.size());
    for (size_t i = 0; i < contour.size(); ++i) {
        poly[i] = contour[i].position;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double px = static_cast<double>(x) + 0.5;
            double py = static_cast<double>(y) + 0.5;

            bool inside = pointInPolygon(px, py, poly);
            uint8_t val = inside ? 255 : 0;

            if (inside && m_feather > 0) {
                double minDist = 1e9;
                for (size_t i = 0; i < poly.size(); ++i) {
                    size_t j = (i + 1) % poly.size();
                    double dx = poly[j].x - poly[i].x;
                    double dy = poly[j].y - poly[i].y;
                    double lenSq = dx * dx + dy * dy;
                    if (lenSq < 1e-6) continue;
                    double t = std::clamp(((px - poly[i].x) * dx + (py - poly[i].y) * dy) / lenSq, 0.0, 1.0);
                    double ex = poly[i].x + t * dx;
                    double ey = poly[i].y + t * dy;
                    double dist = std::sqrt((px - ex) * (px - ex) + (py - ey) * (py - ey));
                    minDist = std::min(minDist, dist);
                }

                if (minDist < m_feather) {
                    val = static_cast<uint8_t>(255.0 * minDist / m_feather);
                }
            }

            uint8_t* px_out = matte.pixelAt(x, y);
            px_out[0] = val;
            px_out[1] = val;
            px_out[2] = val;
            px_out[3] = val;
        }
    }

    return matte;
}

} // namespace FreeEffect
