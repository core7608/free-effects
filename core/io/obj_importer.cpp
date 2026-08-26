#include "obj_importer.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>

namespace FreeEffect {

void OBJModel::computeBounds() {
    boundsMin = {std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max()};
    boundsMax = {std::numeric_limits<float>::lowest(),
                 std::numeric_limits<float>::lowest(),
                 std::numeric_limits<float>::lowest()};

    for (auto& mesh : meshes) {
        for (auto& v : mesh.vertices) {
            boundsMin.x = std::min(boundsMin.x, v.x);
            boundsMin.y = std::min(boundsMin.y, v.y);
            boundsMin.z = std::min(boundsMin.z, v.z);
            boundsMax.x = std::max(boundsMax.x, v.x);
            boundsMax.y = std::max(boundsMax.y, v.y);
            boundsMax.z = std::max(boundsMax.z, v.z);
        }
    }
}

bool OBJImporter::canImport(const std::string& path) const {
    if (path.size() < 4) return false;
    std::string ext = path.substr(path.size() - 4);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == ".obj";
}

OBJModel OBJImporter::import(const std::string& path) {
    OBJModel model;

    std::vector<Vertex> globalVertices;
    std::vector<TexCoord> globalTexCoords;
    std::vector<Normal> globalNormals;

    struct RawFace {
        std::vector<int> vi, ti, ni;
    };
    std::vector<std::pair<RawFace, std::string>> rawFaces;

    std::ifstream file(path);
    if (!file.is_open()) return model;

    std::string currentGroup = "default";
    std::string currentMaterial;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            Vertex v;
            if (iss >> v.x >> v.y >> v.z) {
                globalVertices.push_back(v);
            }
        } else if (prefix == "vt") {
            TexCoord tc;
            tc.u = 0; tc.v = 0;
            if (iss >> tc.u) iss >> tc.v;
            globalTexCoords.push_back(tc);
        } else if (prefix == "vn") {
            Normal n;
            n.nx = 0; n.ny = 0; n.nz = 1;
            iss >> n.nx >> n.ny >> n.nz;
            globalNormals.push_back(n);
        } else if (prefix == "f") {
            RawFace face;
            std::string token;
            while (iss >> token) {
                int vi = -1, ti = -1, ni = -1;
                size_t pos = 0;

                auto parseIndex = [&](int& out) {
                    if (pos >= token.size()) return;
                    size_t start = pos;
                    while (pos < token.size() && token[pos] != '/') pos++;
                    std::string num = token.substr(start, pos - start);
                    if (!num.empty()) {
                        out = std::stoi(num);
                        if (out < 0) {
                            if (prefix == "v" || true) {
                                // Negative indices: -1 = last vertex
                                // We resolve them after we know total counts
                            }
                        }
                    }
                };

                parseIndex(vi);
                if (pos < token.size() && token[pos] == '/') { pos++; parseIndex(ti); }
                if (pos < token.size() && token[pos] == '/') { pos++; parseIndex(ni); }

                face.vi.push_back(vi);
                face.ti.push_back(ti);
                face.ni.push_back(ni);
            }
            rawFaces.push_back({face, currentMaterial});
        } else if (prefix == "g") {
            std::string name;
            std::getline(iss, name);
            if (!name.empty() && name[0] == ' ') name = name.substr(1);
            currentGroup = name.empty() ? "default" : name;
        } else if (prefix == "o") {
            // object name, ignore for grouping
        } else if (prefix == "usemtl") {
            iss >> currentMaterial;
        } else if (prefix == "mtllib") {
            std::string mtlFile;
            std::getline(iss, mtlFile);
            if (!mtlFile.empty() && mtlFile[0] == ' ') mtlFile = mtlFile.substr(1);
            model.materialLibs.push_back(mtlFile);
        }
    }
    file.close();

    // Resolve negative indices
    for (auto& [face, mat] : rawFaces) {
        for (auto& idx : face.vi) {
            if (idx < 0) idx = static_cast<int>(globalVertices.size()) + idx + 1;
        }
        for (auto& idx : face.ti) {
            if (idx < 0) idx = static_cast<int>(globalTexCoords.size()) + idx + 1;
        }
        for (auto& idx : face.ni) {
            if (idx < 0) idx = static_cast<int>(globalNormals.size()) + idx + 1;
        }
    }

    // Group faces by group name
    std::map<std::string, std::vector<std::pair<RawFace, std::string>>> groupedFaces;
    for (auto& [face, mat] : rawFaces) {
        groupedFaces[currentGroup].push_back({face, mat});
    }

    // Build per-mesh data by deduplicating vertices
    for (auto& [groupName, faces] : groupedFaces) {
        OBJMesh mesh;
        mesh.name = groupName;

        struct VertexKey {
            int vi, ti, ni;
            bool operator<(const VertexKey& o) const {
                if (vi != o.vi) return vi < o.vi;
                if (ti != o.ti) return ti < o.ti;
                return ni < o.ni;
            }
        };
        std::map<VertexKey, int> vertexMap;

        for (auto& [face, mat] : faces) {
            mesh.materialName = mat;
            Face f;
            for (size_t i = 0; i < face.vi.size(); ++i) {
                int vi = face.vi[i];
                int ti = (i < face.ti.size()) ? face.ti[i] : -1;
                int ni = (i < face.ni.size()) ? face.ni[i] : -1;

                VertexKey key{vi, ti, ni};
                auto it = vertexMap.find(key);
                if (it != vertexMap.end()) {
                    f.vertexIndices.push_back(it->second);
                } else {
                    int newIndex = static_cast<int>(mesh.vertices.size());
                    vertexMap[key] = newIndex;

                    if (vi > 0 && vi <= static_cast<int>(globalVertices.size())) {
                        mesh.vertices.push_back(globalVertices[vi - 1]);
                    }
                    if (ti > 0 && ti <= static_cast<int>(globalTexCoords.size())) {
                        mesh.texCoords.push_back(globalTexCoords[ti - 1]);
                    }
                    if (ni > 0 && ni <= static_cast<int>(globalNormals.size())) {
                        mesh.normals.push_back(globalNormals[ni - 1]);
                    }
                    f.vertexIndices.push_back(newIndex);
                }
            }
            mesh.faces.push_back(f);
        }

        model.meshes.push_back(std::move(mesh));
    }

    // Handle case with no groups
    if (model.meshes.empty() && !rawFaces.empty()) {
        OBJMesh mesh;
        mesh.name = "default";
        struct VertexKey {
            int vi, ti, ni;
            bool operator<(const VertexKey& o) const {
                if (vi != o.vi) return vi < o.vi;
                if (ti != o.ti) return ti < o.ti;
                return ni < o.ni;
            }
        };
        std::map<VertexKey, int> vertexMap;

        for (auto& [face, mat] : rawFaces) {
            mesh.materialName = mat;
            Face f;
            for (size_t i = 0; i < face.vi.size(); ++i) {
                int vi = face.vi[i];
                int ti = (i < face.ti.size()) ? face.ti[i] : -1;
                int ni = (i < face.ni.size()) ? face.ni[i] : -1;

                VertexKey key{vi, ti, ni};
                auto it = vertexMap.find(key);
                if (it != vertexMap.end()) {
                    f.vertexIndices.push_back(it->second);
                } else {
                    int newIndex = static_cast<int>(mesh.vertices.size());
                    vertexMap[key] = newIndex;

                    if (vi > 0 && vi <= static_cast<int>(globalVertices.size())) {
                        mesh.vertices.push_back(globalVertices[vi - 1]);
                    }
                    if (ti > 0 && ti <= static_cast<int>(globalTexCoords.size())) {
                        mesh.texCoords.push_back(globalTexCoords[ti - 1]);
                    }
                    if (ni > 0 && ni <= static_cast<int>(globalNormals.size())) {
                        mesh.normals.push_back(globalNormals[ni - 1]);
                    }
                    f.vertexIndices.push_back(newIndex);
                }
            }
            mesh.faces.push_back(f);
        }
        model.meshes.push_back(std::move(mesh));
    }

    model.computeBounds();
    return model;
}

void OBJImporter::parseVertex(const std::string& line, OBJModel& model) {
    (void)line; (void)model;
}

void OBJImporter::parseTexCoord(const std::string& line, OBJModel& model) {
    (void)line; (void)model;
}

void OBJImporter::parseNormal(const std::string& line, OBJModel& model) {
    (void)line; (void)model;
}

void OBJImporter::parseFace(const std::string& line, OBJMesh& mesh) {
    (void)line; (void)mesh;
}

void OBJImporter::parseMTL(const std::string& path, OBJModel& model) {
    (void)path; (void)model;
}

bool OBJImporter::parseLine(const std::string& line, OBJModel& model) {
    (void)line; (void)model;
    return false;
}

} // namespace FreeEffect
