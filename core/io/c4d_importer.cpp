#include "c4d_importer.h"
#include <fstream>
#include <cstring>
#include <algorithm>

namespace FreeEffect {

bool C4DImporter::canImport(const std::string& path) const {
    if (path.size() < 4) return false;
    std::string ext = path.substr(path.size() - 4);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == ".c4d";
}

// BinaryReader implementation
bool C4DImporter::BinaryReader::readU32(uint32_t& out) {
    if (offset + 4 > size) return false;
    std::memcpy(&out, data + offset, 4);
    offset += 4;
    return true;
}

bool C4DImporter::BinaryReader::readU16(uint16_t& out) {
    if (offset + 2 > size) return false;
    std::memcpy(&out, data + offset, 2);
    offset += 2;
    return true;
}

bool C4DImporter::BinaryReader::readU8(uint8_t& out) {
    if (offset >= size) return false;
    out = data[offset];
    offset++;
    return true;
}

bool C4DImporter::BinaryReader::readFloat(float& out) {
    if (offset + 4 > size) return false;
    std::memcpy(&out, data + offset, 4);
    offset += 4;
    return true;
}

bool C4DImporter::BinaryReader::readDouble(double& out) {
    if (offset + 8 > size) return false;
    std::memcpy(&out, data + offset, 8);
    offset += 8;
    return true;
}

bool C4DImporter::BinaryReader::readBytes(void* dst, size_t count) {
    if (offset + count > size) return false;
    std::memcpy(dst, data + offset, count);
    offset += count;
    return true;
}

bool C4DImporter::BinaryReader::readString(std::string& out, size_t length) {
    if (offset + length > size) return false;
    out.assign(reinterpret_cast<const char*>(data + offset), length);
    offset += length;
    return true;
}

bool C4DImporter::BinaryReader::eof() const {
    return offset >= size;
}

bool C4DImporter::BinaryReader::skip(size_t count) {
    if (offset + count > size) return false;
    offset += count;
    return true;
}

C4DModel C4DImporter::import(const std::string& path) {
    C4DModel model;
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return model;

    std::streamsize fileSize = file.tellg();
    if (fileSize <= 0) return model;

    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(fileSize));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        return model;
    }
    file.close();

    BinaryReader reader(buffer.data(), buffer.size());
    parseHeader(reader, model);

    if (model.valid && !reader.eof()) {
        parseObjects(reader, model);
    }

    return model;
}

void C4DImporter::parseHeader(BinaryReader& reader, C4DModel& model) {
    if (reader.size < 16) return;

    // C4D files start with a 4-byte magic/header
    uint8_t header[8];
    if (!reader.readBytes(header, 8)) return;

    // Check common C4D magic bytes
    // C4D files typically start with specific patterns
    // The first 4 bytes are often the file signature
    if (header[0] != 'C' || header[1] != '4' || header[2] != 'D') {
        // Try alternative header format
        uint32_t magic;
        std::memcpy(&magic, header, 4);

        // Common C4D version indicators
        if (magic > 100000) {
            model.formatVersion = static_cast<int>(magic);
        } else {
            return;
        }
    } else {
        model.formatVersion = header[3];
    }

    // Read additional header fields
    uint32_t version = 0;
    if (!reader.readU32(version)) return;
    model.formatVersion = static_cast<int>(version);

    // Read identifiers/flags
    uint32_t flags = 0;
    reader.readU32(flags);

    model.valid = true;
}

void C4DImporter::parseObjects(BinaryReader& reader, C4DModel& model) {
    // Parse object blocks until EOF
    while (!reader.eof() && reader.offset + 8 < reader.size) {
        uint32_t blockType = 0;
        uint32_t blockSize = 0;

        if (!reader.readU32(blockType)) break;
        if (!reader.readU32(blockSize)) break;

        // Sanity check block size
        if (blockSize > reader.size - reader.offset) break;

        size_t blockStart = reader.offset;

        switch (blockType) {
            case 0x00001000: // Mesh object
            {
                C4DMesh mesh;
                parseMeshObject(reader, mesh);
                mesh.name = "mesh_" + std::to_string(model.meshes.size());
                model.meshes.push_back(std::move(mesh));
                break;
            }
            case 0x00002000: // Camera object
            {
                C4DCamera cam;
                parseCameraObject(reader, cam);
                cam.name = "camera_" + std::to_string(model.cameras.size());
                model.cameras.push_back(std::move(cam));
                break;
            }
            case 0x00003000: // Light object
            {
                C4DLight light;
                parseLightObject(reader, light);
                light.name = "light_" + std::to_string(model.lights.size());
                model.lights.push_back(std::move(light));
                break;
            }
            case 0x00004000: // Material
            {
                C4DMesh matMesh;
                parseMaterialBlock(reader, matMesh);
                if (!model.meshes.empty()) {
                    auto& lastMesh = model.meshes.back();
                    lastMesh.diffuse[0] = matMesh.diffuse[0];
                    lastMesh.diffuse[1] = matMesh.diffuse[1];
                    lastMesh.diffuse[2] = matMesh.diffuse[2];
                    lastMesh.diffuse[3] = matMesh.diffuse[3];
                    lastMesh.specular[0] = matMesh.specular[0];
                    lastMesh.specular[1] = matMesh.specular[1];
                    lastMesh.specular[2] = matMesh.specular[2];
                    lastMesh.shininess = matMesh.shininess;
                }
                break;
            }
            case 0x00005000: // Animation track
            {
                C4DAnimation anim;
                parseKeyframeTrack(reader, anim);
                anim.nodeName = "object_" + std::to_string(model.animations.size());
                model.animations.push_back(std::move(anim));
                break;
            }
            default:
                // Unknown block type, skip
                break;
        }

        // Ensure we're at the end of the block
        reader.offset = blockStart + blockSize;
    }
}

void C4DImporter::parseMeshObject(BinaryReader& reader, C4DMesh& mesh) {
    uint32_t vertexCount = 0;
    uint32_t polygonCount = 0;

    if (!reader.readU32(vertexCount)) return;
    if (!reader.readU32(polygonCount)) return;

    // Sanity limits
    if (vertexCount > 10000000 || polygonCount > 10000000) return;

    mesh.vertices.resize(vertexCount);
    for (uint32_t i = 0; i < vertexCount; ++i) {
        if (!reader.readFloat(mesh.vertices[i].x)) return;
        if (!reader.readFloat(mesh.vertices[i].y)) return;
        if (!reader.readFloat(mesh.vertices[i].z)) return;
    }

    mesh.polygons.resize(polygonCount);
    for (uint32_t i = 0; i < polygonCount; ++i) {
        uint8_t polySize = 0;
        if (!reader.readU8(polySize)) return;

        mesh.polygons[i].indices.resize(polySize);
        for (uint8_t j = 0; j < polySize; ++j) {
            uint32_t idx;
            if (!reader.readU32(idx)) return;
            mesh.polygons[i].indices[j] = static_cast<int>(idx);
        }
    }

    // Compute simple normals per-vertex
    mesh.normals.resize(vertexCount);
    for (auto& n : mesh.normals) { n.nx = 0; n.ny = 0; n.nz = 1; }

    for (auto& poly : mesh.polygons) {
        if (poly.indices.size() < 3) continue;
        int i0 = poly.indices[0];
        int i1 = poly.indices[1];
        int i2 = poly.indices[2];
        if (i0 < 0 || i0 >= static_cast<int>(vertexCount)) continue;
        if (i1 < 0 || i1 >= static_cast<int>(vertexCount)) continue;
        if (i2 < 0 || i2 >= static_cast<int>(vertexCount)) continue;

        float ax = mesh.vertices[i1].x - mesh.vertices[i0].x;
        float ay = mesh.vertices[i1].y - mesh.vertices[i0].y;
        float az = mesh.vertices[i1].z - mesh.vertices[i0].z;
        float bx = mesh.vertices[i2].x - mesh.vertices[i0].x;
        float by = mesh.vertices[i2].y - mesh.vertices[i0].y;
        float bz = mesh.vertices[i2].z - mesh.vertices[i0].z;

        float nx = ay * bz - az * by;
        float ny = az * bx - ax * bz;
        float nz = ax * by - ay * bx;

        float len = std::sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 1e-8f) { nx /= len; ny /= len; nz /= len; }

        for (int idx : poly.indices) {
            if (idx >= 0 && idx < static_cast<int>(vertexCount)) {
                mesh.normals[idx].nx += nx;
                mesh.normals[idx].ny += ny;
                mesh.normals[idx].nz += nz;
            }
        }
    }

    for (auto& n : mesh.normals) {
        float len = std::sqrt(n.nx * n.nx + n.ny * n.ny + n.nz * n.nz);
        if (len > 1e-8f) { n.nx /= len; n.ny /= len; n.nz /= len; }
    }
}

void C4DImporter::parseCameraObject(BinaryReader& reader, C4DCamera& cam) {
    if (reader.size - reader.offset < 40) return;

    reader.readFloat(cam.position[0]);
    reader.readFloat(cam.position[1]);
    reader.readFloat(cam.position[2]);
    reader.readFloat(cam.target[0]);
    reader.readFloat(cam.target[1]);
    reader.readFloat(cam.target[2]);
    reader.readFloat(cam.focalLength);
    reader.readFloat(cam.nearPlane);
    reader.readFloat(cam.farPlane);
}

void C4DImporter::parseLightObject(BinaryReader& reader, C4DLight& light) {
    if (reader.size - reader.offset < 36) return;

    uint32_t lightType;
    reader.readU32(lightType);
    light.type = static_cast<int>(lightType);

    reader.readFloat(light.position[0]);
    reader.readFloat(light.position[1]);
    reader.readFloat(light.position[2]);
    reader.readFloat(light.color[0]);
    reader.readFloat(light.color[1]);
    reader.readFloat(light.color[2]);
    reader.readFloat(light.intensity);
    reader.readFloat(light.coneAngle);
}

void C4DImporter::parseMaterialBlock(BinaryReader& reader, C4DMesh& mesh) {
    if (reader.size - reader.offset < 16) return;

    reader.readFloat(mesh.diffuse[0]);
    reader.readFloat(mesh.diffuse[1]);
    reader.readFloat(mesh.diffuse[2]);
    mesh.diffuse[3] = 1.0f;
    reader.readFloat(mesh.shininess);
}

void C4DImporter::parseKeyframeTrack(BinaryReader& reader, C4DAnimation& anim) {
    uint32_t keyCount = 0;
    reader.readU32(keyCount);

    if (keyCount > 100000) return;

    anim.keyframes.resize(keyCount);
    for (uint32_t i = 0; i < keyCount; ++i) {
        float time, value;
        reader.readFloat(time);
        reader.readFloat(value);
        anim.keyframes[i].time = static_cast<double>(time);
        anim.keyframes[i].value = value;
    }
}

} // namespace FreeEffect
