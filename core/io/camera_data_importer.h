#pragma once

#include <string>
#include <vector>
#include "../timeline/camera.h"
#include "../timeline/light.h"

namespace FreeEffect {

struct CameraKeyframe {
    double time = 0;
    double position[3] = {0, 0, 0};
    double target[3] = {0, 0, -1};
    double up[3] = {0, 1, 0};
    double fov = 50.0;
};

struct LightKeyframe {
    double time = 0;
    double position[3] = {0, 0, 0};
    double target[3] = {0, 0, -1};
    double color[3] = {1, 1, 1};
    double intensity = 1.0;
    std::string lightType = "Point";
};

struct CameraData {
    std::string sourceApp;
    std::vector<CameraKeyframe> cameraKeyframes;
    std::vector<LightKeyframe> lightKeyframes;
    double frameRate = 30.0;
};

class CameraDataImporter {
public:
    CameraData importFromJSON(const std::string& path);
    CameraData importFromFBX(const std::string& path);
    CameraData importFromDAE(const std::string& path);
    CameraData importFromABC(const std::string& path);

    bool exportToAE(const CameraData& data, const std::string& path);

private:
    bool parseFBXCamera(const std::string& path, CameraData& data);
    bool parseDAECamera(const std::string& path, CameraData& data);
    bool parseABCCamera(const std::string& path, CameraData& data);

    static std::string readFile(const std::string& path);
    static std::string extractTagValue(const std::string& content, const std::string& tag);
    static std::string extractNestedTag(const std::string& content, const std::string& openTag, const std::string& closeTag);
    static double parseDouble(const std::string& s, double def = 0.0);
    static int parseInt(const std::string& s, int def = 0);
};

} // namespace FreeEffect
