#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "../timeline/camera.h"
#include "../rendering/renderer.h"

namespace FreeEffect {

struct TrackingPoint {
    float x, y;
    float confidence = 0;
    int trackIndex = -1;
    float worldX = 0, worldY = 0, worldZ = 0;
};

struct CameraSolve {
    Camera camera;
    std::vector<TrackingPoint> points;
    double error = 0;
    bool valid = false;
};

class CameraTracker {
public:
    void setFeatureCount(int count) { m_featureCount = count; }
    void setMinTrackLength(int length) { m_minTrackLength = length; }
    
    void analyzeFootage(const std::vector<PixelBuffer>& frames, double fps);
    
    CameraSolve solveCamera(int width, int height, double focalLength);
    
    const std::vector<TrackingPoint>& getTrackingPoints() const;
    
    void createTexturedSolid(const TrackingPoint& point, int width, int height);

private:
    int m_featureCount = 300;
    int m_minTrackLength = 10;
    std::vector<TrackingPoint> m_trackingPoints;
    std::vector<std::vector<std::pair<int, float>>> m_trackHistories;

    void detectFeatures(const PixelBuffer& frame, std::vector<TrackingPoint>& features);
    void trackFeatures(const std::vector<PixelBuffer>& frames);
    void computeFundamentalMatrix(const std::vector<TrackingPoint>& pointsA,
                                   const std::vector<TrackingPoint>& pointsB,
                                   float F[3][3]);
    void decomposeEssentialMatrix(const float E[3][3], float R[3][3], float t[3]);
    void triangulatePoints(const float R1[3][3], const float t1[3],
                           const float R2[3][3], const float t2[3],
                           const std::vector<TrackingPoint>& pts1,
                           const std::vector<TrackingPoint>& pts2,
                           std::vector<TrackingPoint>& worldPoints);

    float computeGrayscale(const uint8_t* pixel) const;
    float getPixelGray(const PixelBuffer& buf, int x, int y) const;
    float sampleBilinear(const PixelBuffer& buf, float x, float y) const;
};

} // namespace FreeEffect
