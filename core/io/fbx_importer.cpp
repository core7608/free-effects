#include "fbx_importer.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace FreeEffect {

bool FBXImporter::canImport(const std::string& path) const {
    if (path.size() < 4) return false;
    std::string ext = path.substr(path.size() - 4);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == ".fbx";
}

FBXModel FBXImporter::import(const std::string& path) {
    FBXModel model;
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return model;

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    if (content.empty()) return model;

    // Check for FBX magic
    if (content.substr(0, 27) == "Kaydara FBX Binary  \x00") {
        return model;
    }

    auto tokens = tokenize(content);
    if (tokens.empty()) return model;

    size_t pos = 0;

    // Parse header
    while (pos < tokens.size()) {
        if (tokens[pos].type == Token::IDENTIFIER && tokens[pos].value == "FBXHeaderExtension") {
            pos++;
            if (pos < tokens.size() && tokens[pos].type == Token::LBRACE) {
                pos++;
                while (pos < tokens.size() && tokens[pos].type != Token::RBRACE) {
                    if (tokens[pos].value == "FBXHeaderVersion" && pos + 1 < tokens.size()) {
                        pos++;
                        if (pos < tokens.size()) model.versionMajor = std::stoi(tokens[pos].value);
                    } else if (tokens[pos].value == "FBXVersion" && pos + 1 < tokens.size()) {
                        pos++;
                        if (pos < tokens.size()) model.versionMinor = std::stoi(tokens[pos].value);
                    } else {
                        pos++;
                    }
                }
                if (pos < tokens.size()) pos++;
            }
        } else if (tokens[pos].type == Token::IDENTIFIER) {
            parseNode(tokens, pos, 0, model);
        } else {
            pos++;
        }
    }

    return model;
}

std::vector<FBXImporter::Token> FBXImporter::tokenize(const std::string& content) {
    std::vector<Token> tokens;
    size_t i = 0;
    size_t len = content.size();

    while (i < len) {
        if (content[i] == ';' || content[i] == '\n' || content[i] == '\r') {
            i++;
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(content[i]))) {
            i++;
            continue;
        }
        if (content[i] == '{') {
            tokens.push_back({Token::LBRACE, "{"});
            i++;
            continue;
        }
        if (content[i] == '}') {
            tokens.push_back({Token::RBRACE, "}"});
            i++;
            continue;
        }
        if (content[i] == ',') {
            tokens.push_back({Token::COMMA, ","});
            i++;
            continue;
        }
        if (content[i] == ':') {
            tokens.push_back({Token::COLON, ":"});
            i++;
            continue;
        }
        if (content[i] == '*') {
            tokens.push_back({Token::STAR, "*"});
            i++;
            continue;
        }
        if (content[i] == '"') {
            i++;
            std::string str;
            while (i < len && content[i] != '"') {
                str += content[i];
                i++;
            }
            if (i < len) i++;
            tokens.push_back({Token::STRING, str});
            continue;
        }
        if (content[i] == '-' || std::isdigit(static_cast<unsigned char>(content[i]))) {
            std::string num;
            num += content[i];
            i++;
            while (i < len && (std::isdigit(static_cast<unsigned char>(content[i])) || content[i] == '.' || content[i] == 'e' || content[i] == 'E' || content[i] == '+' || content[i] == '-')) {
                num += content[i];
                i++;
            }
            if (num.find('.') != std::string::npos || num.find('e') != std::string::npos || num.find('E') != std::string::npos) {
                tokens.push_back({Token::FLOAT, num});
            } else {
                tokens.push_back({Token::INTEGER, num});
            }
            continue;
        }
        if (std::isalpha(static_cast<unsigned char>(content[i])) || content[i] == '_') {
            std::string id;
            while (i < len && (std::isalnum(static_cast<unsigned char>(content[i])) || content[i] == '_' || content[i] == '.' || content[i] == '-')) {
                id += content[i];
                i++;
            }
            tokens.push_back({Token::IDENTIFIER, id});
            continue;
        }
        i++;
    }

    tokens.push_back({Token::END_OF_FILE, ""});
    return tokens;
}

std::string FBXImporter::getTokenValue(const std::vector<Token>& tokens, size_t& pos) {
    if (pos >= tokens.size()) return "";
    return tokens[pos++].value;
}

std::vector<float> FBXImporter::getFloatArray(const std::vector<Token>& tokens, size_t& pos) {
    std::vector<float> result;
    if (pos >= tokens.size()) return result;

    // Expect *N format for array length
    if (tokens[pos].type == Token::STAR) {
        pos++;
        if (pos < tokens.size()) {
            int count = std::stoi(tokens[pos].value);
            pos++;
            // Skip the opening brace of array data if present
            result.reserve(count);
            while (pos < tokens.size() && tokens[pos].type != Token::RBRACE &&
                   static_cast<int>(result.size()) < count) {
                if (tokens[pos].type == Token::FLOAT || tokens[pos].type == Token::INTEGER) {
                    result.push_back(std::stof(tokens[pos].value));
                }
                pos++;
            }
        }
    } else {
        while (pos < tokens.size() && tokens[pos].type != Token::RBRACE &&
               tokens[pos].type != Token::IDENTIFIER) {
            if (tokens[pos].type == Token::COMMA) { pos++; continue; }
            if (tokens[pos].type == Token::FLOAT || tokens[pos].type == Token::INTEGER) {
                result.push_back(std::stof(tokens[pos].value));
            }
            pos++;
        }
    }
    return result;
}

void FBXImporter::parseNode(const std::vector<Token>& tokens, size_t& pos, int depth, FBXModel& model) {
    if (pos >= tokens.size() || tokens[pos].type != Token::IDENTIFIER) return;

    std::string name = tokens[pos].value;
    pos++;

    // Skip properties after colon
    while (pos < tokens.size() && tokens[pos].type != Token::LBRACE &&
           tokens[pos].type != Token::RBRACE && tokens[pos].type != Token::END_OF_FILE) {
        pos++;
    }

    if (pos >= tokens.size() || tokens[pos].type != Token::LBRACE) return;
    pos++;

    // Store start position for mesh data parsing
    size_t contentStart = pos;

    // Count nested braces
    int braceCount = 1;
    while (pos < tokens.size() && braceCount > 0) {
        if (tokens[pos].type == Token::LBRACE) braceCount++;
        else if (tokens[pos].type == Token::RBRACE) braceCount--;
        if (braceCount > 0) pos++;
    }
    size_t contentEnd = pos;

    if (pos < tokens.size()) pos++; // skip closing brace

    // Parse children within contentStart..contentEnd
    size_t innerPos = contentStart;
    while (innerPos < contentEnd) {
        if (tokens[innerPos].type == Token::IDENTIFIER) {
            std::string childName = tokens[innerPos].value;
            innerPos++;

            // Skip child properties
            while (innerPos < contentEnd && tokens[innerPos].type != Token::LBRACE) innerPos++;
            if (innerPos < contentEnd) innerPos++; // skip LBRACE

            // Count braces
            int bc = 1;
            size_t childStart = innerPos;
            while (innerPos < contentEnd && bc > 0) {
                if (tokens[innerPos].type == Token::LBRACE) bc++;
                else if (tokens[innerPos].type == Token::RBRACE) bc--;
                if (bc > 0) innerPos++;
            }
            size_t childEnd = innerPos;
            if (innerPos < contentEnd) innerPos++;

            // Dispatch based on node type
            if (childName == "Geometry") {
                FBXMeshNode mesh;
                mesh.name = "mesh_" + std::to_string(model.meshes.size());
                std::memset(mesh.transform, 0, sizeof(mesh.transform));
                mesh.transform[0] = mesh.transform[5] = mesh.transform[10] = mesh.transform[15] = 1.0f;

                size_t mpos = childStart;
                while (mpos < childEnd) {
                    if (tokens[mpos].type == Token::IDENTIFIER) {
                        std::string prop = tokens[mpos].value;
                        mpos++;
                        while (mpos < childEnd && tokens[mpos].type != Token::LBRACE &&
                               tokens[mpos].type != Token::IDENTIFIER) mpos++;

                        if (prop == "Vertices") {
                            auto verts = getFloatArray(tokens, mpos);
                            for (size_t i = 0; i + 2 < verts.size(); i += 3) {
                                mesh.vertices.push_back({verts[i], verts[i + 1], verts[i + 2]});
                            }
                        } else if (prop == "PolygonVertexIndex") {
                            auto indices = getFloatArray(tokens, mpos);
                            FBXPolygon poly;
                            for (auto idx : indices) {
                                int i = static_cast<int>(idx);
                                poly.indices.push_back(i);
                            }
                            mesh.polygons.push_back(poly);
                        } else if (prop == "Normals") {
                            auto norms = getFloatArray(tokens, mpos);
                            for (size_t i = 0; i + 2 < norms.size(); i += 3) {
                                mesh.normals.push_back({norms[i], norms[i + 1], norms[i + 2]});
                            }
                        } else if (prop == "UV") {
                            auto uvs = getFloatArray(tokens, mpos);
                            for (size_t i = 0; i + 1 < uvs.size(); i += 2) {
                                mesh.texCoords.push_back({uvs[i], uvs[i + 1]});
                            }
                        } else {
                            mpos++;
                        }
                    } else {
                        mpos++;
                    }
                }
                if (!mesh.vertices.empty()) {
                    model.meshes.push_back(mesh);
                }
            } else if (childName == "Material") {
                FBXMaterial mat;
                size_t mpos = childStart;
                while (mpos < childEnd) {
                    if (tokens[mpos].type == Token::IDENTIFIER) {
                        std::string prop = tokens[mpos].value;
                        mpos++;
                        while (mpos < childEnd && tokens[mpos].type != Token::LBRACE &&
                               tokens[mpos].type != Token::IDENTIFIER) mpos++;
                        if (mpos < childEnd && tokens[mpos].type == Token::LBRACE) mpos++;

                        if (prop == "Properties70" || prop == "Properties60") {
                            int bc = 1;
                            while (mpos < childEnd && bc > 0) {
                                if (tokens[mpos].type == Token::LBRACE) bc++;
                                else if (tokens[mpos].type == Token::RBRACE) bc--;
                                if (bc > 0) mpos++;
                            }
                            if (mpos < childEnd) mpos++;
                        } else {
                            mpos++;
                        }
                    } else {
                        mpos++;
                    }
                }
                mat.name = "material_" + std::to_string(model.materials.size());
                model.materials.push_back(mat);
            } else if (childName == "Camera") {
                FBXCamera cam;
                cam.name = "camera_" + std::to_string(model.cameras.size());
                model.cameras.push_back(cam);
            } else if (childName == "Light") {
                FBXLightNode light;
                light.name = "light_" + std::to_string(model.lights.size());
                model.lights.push_back(light);
            }
        } else {
            innerPos++;
        }
    }
}

void FBXImporter::parseGeometry(const std::vector<Token>& tokens, size_t& pos, FBXMeshNode& mesh) {
    (void)tokens; (void)pos; (void)mesh;
}

void FBXImporter::parseMaterial(const std::vector<Token>& tokens, size_t& pos, FBXMaterial& mat) {
    (void)tokens; (void)pos; (void)mat;
}

void FBXImporter::parseCamera(const std::vector<Token>& tokens, size_t& pos, FBXCamera& cam) {
    (void)tokens; (void)pos; (void)cam;
}

void FBXImporter::parseLight(const std::vector<Token>& tokens, size_t& pos, FBXLightNode& light) {
    (void)tokens; (void)pos; (void)light;
}

void FBXImporter::parseAnimationStack(const std::vector<Token>& tokens, size_t& pos, FBXModel& model) {
    (void)tokens; (void)pos; (void)model;
}

} // namespace FreeEffect
