#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace FreeEffect {

struct C4DVertex { float x, y, z; };
struct C4DNormal { float nx, ny, nz; };
struct C4DPolygon { std::vector<int> indices; };
struct C4DUV { float u, v; };

struct C4DMesh {
    std::string name;
    std::vector<C4DVertex> vertices;
    std::vector<C4DNormal> normals;
    std::vector<C4DPolygon> polygons;
    std::vector<C4DUV> uvCoords;
    float diffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float specular[4] = {0.5f, 0.5f, 0.5f, 1.0f};
    float shininess = 32.0f;
};

struct C4DCamera {
    std::string name;
    float position[3] = {0, 0, -5};
    float target[3] = {0, 0, 0};
    float focalLength = 36.0f;
    float nearPlane = 1.0f;
    float farPlane = 100000.0f;
};

struct C4DLight {
    std::string name;
    int type = 0;
    float position[3] = {0, 0, 0};
    float color[3] = {1, 1, 1};
    float intensity = 1.0f;
    float radius = 0.0f;
    float outerRadius = 0.0f;
    float coneAngle = 45.0f;
    float coneFalloff = 10.0f;
    bool shadows = true;
};

struct C4DKeyframe {
    double time;
    float value;
};

struct C4DAnimation {
    std::string nodeName;
    std::string property;
    std::vector<C4DKeyframe> keyframes;
};

struct C4DModel {
    std::vector<C4DMesh> meshes;
    std::vector<C4DCamera> cameras;
    std::vector<C4DLight> lights;
    std::vector<C4DAnimation> animations;
    int formatVersion = 0;
    bool valid = false;
};

class C4DImporter {
public:
    C4DModel import(const std::string& path);
    bool canImport(const std::string& path) const;

private:
    struct BinaryReader {
        const uint8_t* data;
        size_t size;
        size_t offset = 0;

        BinaryReader(const uint8_t* d, size_t s) : data(d), size(s), offset(0) {}

        bool readU32(uint32_t& out);
        bool readU16(uint16_t& out);
        bool readU8(uint8_t& out);
        bool readFloat(float& out);
        bool readDouble(double& out);
        bool readBytes(void* dst, size_t count);
        bool readString(std::string& out, size_t length);
        bool eof() const;
        bool skip(size_t count);
    };

    void parseHeader(BinaryReader& reader, C4DModel& model);
    void parseObjects(BinaryReader& reader, C4DModel& model);
    void parseMeshObject(BinaryReader& reader, C4DMesh& mesh);
    void parseCameraObject(BinaryReader& reader, C4DCamera& cam);
    void parseLightObject(BinaryReader& reader, C4DLight& light);
    void parseMaterialBlock(BinaryReader& reader, C4DMesh& mesh);
    void parseKeyframeTrack(BinaryReader& reader, C4DAnimation& anim);
};

} // namespace FreeEffect
