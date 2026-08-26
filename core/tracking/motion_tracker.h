#pragma once
#include "../timeline/types.h"
#include <vector>

namespace FreeEffect {

struct TrackPoint {
    double x = 0, y = 0;
    double confidence = 1.0;
};

struct TrackResult {
    double time = 0;
    TrackPoint position;
    double scale = 1.0;
    double rotation = 0;
};

enum class TrackMode { Position, Scale, Rotation, Perspective };
enum class TrackChannel { RGB, Red, Green, Blue, Luminance };

class MotionTracker {
public:
    void setSourceRegion(double x, double y, double w, double h);
    void setTrackChannel(TrackChannel ch) { m_channel = ch; }
    void setTrackMode(TrackMode mode) { m_mode = mode; }
    
    TrackResult trackForward(const uint8_t* frame, int width, int height, double time);
    TrackResult trackBackward(const uint8_t* frame, int width, int height, double time);
    
    const std::vector<TrackResult>& getTrackData() const { return m_results; }
    void clearResults() { m_results.clear(); }
    
    std::vector<std::pair<double,double>> stabilize(int smoothPixels = 10) const;

private:
    TrackMode m_mode = TrackMode::Position;
    TrackChannel m_channel = TrackChannel::Luminance;
    double m_regionX = 0, m_regionY = 0, m_regionW = 50, m_regionH = 50;
    std::vector<TrackResult> m_results;
    
    double computeNCC(const uint8_t* search, const uint8_t* template_, 
                      int sw, int tw, int regionW, int regionH) const;
    double sampleChannel(const uint8_t* pixel) const;
    void extractTemplate(const uint8_t* frame, int width, int height,
                         int cx, int cy, std::vector<uint8_t>& tmpl, int& tw, int& th) const;
};

} // namespace FreeEffect
