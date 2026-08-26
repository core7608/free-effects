#pragma once

#include <string>
#include <vector>

namespace FreeEffect {

struct Vertex { float x, y, z; };
struct TexCoord { float u, v; };
struct Normal { float nx, ny, nz; };
struct Face { std::vector<int> vertexIndices; std::vector<int> texIndices; std::vector<int> normalIndices; };

struct OBJMesh {
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<TexCoord> texCoords;
    std::vector<Normal> normals;
    std::vector<Face> faces;
    std::string materialName;
};

struct OBJModel {
    std::vector<OBJMesh> meshes;
    std::vector<std::string> materialLibs;
    Vertex boundsMin, boundsMax;
    void computeBounds();
};

class OBJImporter {
public:
    OBJModel import(const std::string& path);
    bool canImport(const std::string& path) const;
private:
    bool parseLine(const std::string& line, OBJModel& model);
    void parseVertex(const std::string& line, OBJModel& model);
    void parseTexCoord(const std::string& line, OBJModel& model);
    void parseNormal(const std::string& line, OBJModel& model);
    void parseFace(const std::string& line, OBJMesh& mesh);
    void parseMTL(const std::string& path, OBJModel& model);
};

} // namespace FreeEffect
