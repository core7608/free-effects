#pragma once

#include <string>
#include <vector>
#include <map>

namespace FreeEffect {

struct FBXVertex { float x, y, z; };
struct FBXNormal { float nx, ny, nz; };
struct FBXTexCoord { float u, v; };
struct FBXPolygon { std::vector<int> indices; };

struct FBXMeshNode {
    std::string name;
    std::vector<FBXVertex> vertices;
    std::vector<FBXNormal> normals;
    std::vector<FBXTexCoord> texCoords;
    std::vector<FBXPolygon> polygons;
    int materialIndex = -1;
    double transform[16];
};

struct FBXMaterial {
    std::string name;
    float diffuse[3] = {1.0f, 1.0f, 1.0f};
    float specular[3] = {0.5f, 0.5f, 0.5f};
    float ambient[3] = {0.1f, 0.1f, 0.1f};
    float shininess = 32.0f;
    float opacity = 1.0f;
    std::string diffuseTexture;
};

struct FBXCamera {
    std::string name;
    float position[3] = {0, 0, -5};
    float target[3] = {0, 0, 0};
    float up[3] = {0, 1, 0};
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 10000.0f;
};

struct FBXLightNode {
    std::string name;
    int type = 0;
    float position[3] = {0, 0, 0};
    float color[3] = {1, 1, 1};
    float intensity = 1.0f;
    float coneAngle = 90.0f;
    float coneFalloff = 50.0f;
    bool castShadows = true;
};

struct FBXKeyframe {
    double time;
    float value;
};

struct FBXAnimationCurve {
    std::string nodeName;
    std::string property;
    std::vector<FBXKeyframe> keyframes;
};

struct FBXModel {
    std::vector<FBXMeshNode> meshes;
    std::vector<FBXMaterial> materials;
    std::vector<FBXCamera> cameras;
    std::vector<FBXLightNode> lights;
    std::vector<FBXAnimationCurve> animationCurves;
    int versionMajor = 7;
    int versionMinor = 4;
};

class FBXImporter {
public:
    FBXModel import(const std::string& path);
    bool canImport(const std::string& path) const;

private:
    struct Token {
        enum Type { IDENTIFIER, INTEGER, FLOAT, STRING, LBRACE, RBRACE, COMMA, COLON, STAR, END_OF_FILE };
        Type type;
        std::string value;
    };

    std::vector<Token> tokenize(const std::string& content);
    void parseTokens(const std::vector<Token>& tokens, FBXModel& model);
    void parseNode(const std::vector<Token>& tokens, size_t& pos, int depth, FBXModel& model);
    void parseGeometry(const std::vector<Token>& tokens, size_t& pos, FBXMeshNode& mesh);
    void parseMaterial(const std::vector<Token>& tokens, size_t& pos, FBXMaterial& mat);
    void parseCamera(const std::vector<Token>& tokens, size_t& pos, FBXCamera& cam);
    void parseLight(const std::vector<Token>& tokens, size_t& pos, FBXLightNode& light);
    void parseAnimationStack(const std::vector<Token>& tokens, size_t& pos, FBXModel& model);
    std::string getTokenValue(const std::vector<Token>& tokens, size_t& pos);
    std::vector<float> getFloatArray(const std::vector<Token>& tokens, size_t& pos);
};

} // namespace FreeEffect
