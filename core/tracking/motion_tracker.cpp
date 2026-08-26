#include "motion_tracker.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace FreeEffect {

void MotionTracker::setSourceRegion(double x, double y, double w, double h) {
    m_regionX = x;
    m_regionY = y;
    m_regionW = w;
    m_regionH = h;
}

double MotionTracker::sampleChannel(const uint8_t* pixel) const {
    switch (m_channel) {
        case TrackChannel::Red:      return static_cast<double>(pixel[0]);
        case TrackChannel::Green:    return static_cast<double>(pixel[1]);
        case TrackChannel::Blue:     return static_cast<double>(pixel[2]);
        case TrackChannel::Luminance: return 0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2];
        case TrackChannel::RGB:
        default:
            return (static_cast<double>(pixel[0]) + pixel[1] + pixel[2]) / 3.0;
    }
}

void MotionTracker::extractTemplate(const uint8_t* frame, int width, int height,
                                     int cx, int cy, std::vector<uint8_t>& tmpl, int& tw, int& th) const {
    tw = static_cast<int>(m_regionW);
    th = static_cast<int>(m_regionH);
    tmpl.resize(tw * th);

    int startX = cx - tw / 2;
    int startY = cy - th / 2;

    for (int y = 0; y < th; ++y) {
        for (int x = 0; x < tw; ++x) {
            int sx = std::clamp(startX + x, 0, width - 1);
            int sy = std::clamp(startY + y, 0, height - 1);
            const uint8_t* px = frame + (sy * width + sx) * 4;
            tmpl[y * tw + x] = static_cast<uint8_t>(sampleChannel(px));
        }
    }
}

double MotionTracker::computeNCC(const uint8_t* search, const uint8_t* template_,
                                  int sw, int tw, int regionW, int regionH) const {
    double sumT = 0, sumS = 0;
    double sumT2 = 0, sumS2 = 0;
    int count = regionW * regionH;

    for (int i = 0; i < count; ++i) {
        double t = template_[i];
        double s = search[i];
        sumT += t;
        sumS += s;
        sumT2 += t * t;
        sumS2 += s * s;
    }

    double meanT = sumT / count;
    double meanS = sumS / count;

    double varT = sumT2 - meanT * meanT * count;
    double varS = sumS2 - meanS * meanS * count;
    if (varT < 1e-6 || varS < 1e-6) return 0.0;

    double cov = 0;
    for (int i = 0; i < count; ++i) {
        cov += (template_[i] - meanT) * (search[i] - meanS);
    }

    double denom = std::sqrt(varT * varS);
    if (denom < 1e-6) return 0.0;
    return cov / denom;
}

TrackResult MotionTracker::trackForward(const uint8_t* frame, int width, int height, double time) {
    TrackResult result;
    result.time = time;

    double prevX = m_regionX + m_regionW / 2.0;
    double prevY = m_regionY + m_regionH / 2.0;

    if (!m_results.empty()) {
        prevX = m_results.back().position.x;
        prevY = m_results.back().position.y;
    }

    std::vector<uint8_t> tmpl;
    int tw, th;
    extractTemplate(frame, width, height, static_cast<int>(prevX), static_cast<int>(prevY), tmpl, tw, th);

    int searchRadius = static_cast<int>(std::max(m_regionW, m_regionH) * 1.5);
    double bestNCC = -1.0;
    double bestX = prevX;
    double bestY = prevY;

    std::vector<uint8_t> searchPatch(tw * th);

    for (int dy = -searchRadius; dy <= searchRadius; dy += 2) {
        for (int dx = -searchRadius; dx <= searchRadius; dx += 2) {
            int cx = static_cast<int>(prevX) + dx;
            int cy = static_cast<int>(prevY) + dy;

            int startX = cx - tw / 2;
            int startY = cy - th / 2;

            bool outOfBounds = false;
            for (int y = 0; y < th && !outOfBounds; ++y) {
                for (int x = 0; x < tw && !outOfBounds; ++x) {
                    int sx = startX + x;
                    int sy = startY + y;
                    if (sx < 0 || sx >= width || sy < 0 || sy >= height) {
                        outOfBounds = true;
                        break;
                    }
                    const uint8_t* px = frame + (sy * width + sx) * 4;
                    searchPatch[y * tw + x] = static_cast<uint8_t>(sampleChannel(px));
                }
            }

            if (outOfBounds) continue;

            double ncc = computeNCC(searchPatch.data(), tmpl.data(), tw, tw, tw, th);
            if (ncc > bestNCC) {
                bestNCC = ncc;
                bestX = static_cast<double>(cx);
                bestY = static_cast<double>(cy);
            }
        }
    }

    result.position.x = bestX;
    result.position.y = bestY;
    result.position.confidence = std::max(0.0, bestNCC);
    result.scale = 1.0;
    result.rotation = 0;

    m_regionX = bestX - m_regionW / 2.0;
    m_regionY = bestY - m_regionH / 2.0;
    m_results.push_back(result);

    return result;
}

TrackResult MotionTracker::trackBackward(const uint8_t* frame, int width, int height, double time) {
    TrackResult result;
    result.time = time;

    double prevX = m_regionX + m_regionW / 2.0;
    double prevY = m_regionY + m_regionH / 2.0;

    if (!m_results.empty()) {
        prevX = m_results.front().position.x;
        prevY = m_results.front().position.y;
    }

    std::vector<uint8_t> tmpl;
    int tw, th;
    extractTemplate(frame, width, height, static_cast<int>(prevX), static_cast<int>(prevY), tmpl, tw, th);

    int searchRadius = static_cast<int>(std::max(m_regionW, m_regionH) * 1.5);
    double bestNCC = -1.0;
    double bestX = prevX;
    double bestY = prevY;

    std::vector<uint8_t> searchPatch(tw * th);

    for (int dy = -searchRadius; dy <= searchRadius; dy += 2) {
        for (int dx = -searchRadius; dx <= searchRadius; dx += 2) {
            int cx = static_cast<int>(prevX) + dx;
            int cy = static_cast<int>(prevY) + dy;

            int startX = cx - tw / 2;
            int startY = cy - th / 2;

            bool outOfBounds = false;
            for (int y = 0; y < th && !outOfBounds; ++y) {
                for (int x = 0; x < tw && !outOfBounds; ++x) {
                    int sx = startX + x;
                    int sy = startY + y;
                    if (sx < 0 || sx >= width || sy < 0 || sy >= height) {
                        outOfBounds = true;
                        break;
                    }
                    const uint8_t* px = frame + (sy * width + sx) * 4;
                    searchPatch[y * tw + x] = static_cast<uint8_t>(sampleChannel(px));
                }
            }

            if (outOfBounds) continue;

            double ncc = computeNCC(searchPatch.data(), tmpl.data(), tw, tw, tw, th);
            if (ncc > bestNCC) {
                bestNCC = ncc;
                bestX = static_cast<double>(cx);
                bestY = static_cast<double>(cy);
            }
        }
    }

    result.position.x = bestX;
    result.position.y = bestY;
    result.position.confidence = std::max(0.0, bestNCC);
    result.scale = 1.0;
    result.rotation = 0;

    m_regionX = bestX - m_regionW / 2.0;
    m_regionY = bestY - m_regionH / 2.0;
    m_results.insert(m_results.begin(), result);

    return result;
}

std::vector<std::pair<double,double>> MotionTracker::stabilize(int smoothPixels) const {
    std::vector<std::pair<double,double>> offsets;
    if (m_results.empty()) return offsets;

    offsets.resize(m_results.size(), {0.0, 0.0});

    double avgX = 0, avgY = 0;
    for (const auto& r : m_results) {
        avgX += r.position.x;
        avgY += r.position.y;
    }
    avgX /= m_results.size();
    avgY /= m_results.size();

    double smoothedX = avgX;
    double smoothedY = avgY;

    for (size_t i = 0; i < m_results.size(); ++i) {
        double targetX = m_results[i].position.x;
        double targetY = m_results[i].position.y;

        double alpha = 1.0 / (1.0 + smoothPixels);
        smoothedX = smoothedX + alpha * (targetX - smoothedX);
        smoothedY = smoothedY + alpha * (targetY - smoothedY);

        offsets[i].first = smoothedX - targetX;
        offsets[i].second = smoothedY - targetY;
    }

    return offsets;
}

} // namespace FreeEffect
