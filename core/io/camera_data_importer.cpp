#include "camera_data_importer.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace FreeEffect {

std::string CameraDataImporter::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string CameraDataImporter::extractTagValue(const std::string& content, const std::string& tag) {
    size_t pos = content.find(tag);
    if (pos == std::string::npos) return "";
    pos += tag.size();
    size_t end = pos;
    while (end < content.size() && content[end] != '\n' && content[end] != '\r' && content[end] != ',' && content[end] != '}') {
        ++end;
    }
    std::string val = content.substr(pos, end - pos);
    // Trim
    size_t s = val.find_first_not_of(" \t:=\"'");
    if (s != std::string::npos) val = val.substr(s);
    size_t e = val.find_last_not_of(" \t:=\"'");
    if (e != std::string::npos) val = val.substr(0, e + 1);
    return val;
}

std::string CameraDataImporter::extractNestedTag(const std::string& content, const std::string& openTag, const std::string& closeTag) {
    size_t start = content.find(openTag);
    if (start == std::string::npos) return "";
    start += openTag.size();
    size_t end = content.find(closeTag, start);
    if (end == std::string::npos) return "";
    return content.substr(start, end - start);
}

double CameraDataImporter::parseDouble(const std::string& s, double def) {
    try { return std::stod(s); }
    catch (...) { return def; }
}

int CameraDataImporter::parseInt(const std::string& s, int def) {
    try { return std::stoi(s); }
    catch (...) { return def; }
}

CameraData CameraDataImporter::importFromJSON(const std::string& path) {
    CameraData data;
    std::string content = readFile(path);
    if (content.empty()) return data;

    data.sourceApp = extractTagValue(content, "\"sourceApp\"");
    if (data.sourceApp.empty()) data.sourceApp = "Unknown";

    std::string frStr = extractTagValue(content, "\"frameRate\"");
    if (!frStr.empty()) data.frameRate = parseDouble(frStr, 30.0);

    // Parse camera keyframes array
    size_t camArrayPos = content.find("\"cameraKeyframes\"");
    if (camArrayPos != std::string::npos) {
        size_t arrStart = content.find('[', camArrayPos);
        size_t arrEnd = content.find(']', arrStart);
        if (arrStart != std::string::npos && arrEnd != std::string::npos) {
            std::string arrContent = content.substr(arrStart + 1, arrEnd - arrStart - 1);
            size_t objStart = 0;
            while ((objStart = arrContent.find('{', objStart)) != std::string::npos) {
                size_t objEnd = arrContent.find('}', objStart);
                if (objEnd == std::string::npos) break;
                std::string obj = arrContent.substr(objStart, objEnd - objStart + 1);

                CameraKeyframe kf;
                std::string t = extractTagValue(obj, "\"time\"");
                if (!t.empty()) kf.time = parseDouble(t);

                // Parse position array
                size_t posArr = obj.find("\"position\"");
                if (posArr != std::string::npos) {
                    size_t pStart = obj.find('[', posArr);
                    size_t pEnd = obj.find(']', pStart);
                    if (pStart != std::string::npos && pEnd != std::string::npos) {
                        std::string pv = obj.substr(pStart + 1, pEnd - pStart - 1);
                        sscanf(pv.c_str(), "%lf,%lf,%lf", &kf.position[0], &kf.position[1], &kf.position[2]);
                    }
                }

                // Parse target array
                size_t tgtArr = obj.find("\"target\"");
                if (tgtArr != std::string::npos) {
                    size_t tStart = obj.find('[', tgtArr);
                    size_t tEnd = obj.find(']', tStart);
                    if (tStart != std::string::npos && tEnd != std::string::npos) {
                        std::string tv = obj.substr(tStart + 1, tEnd - tStart - 1);
                        sscanf(tv.c_str(), "%lf,%lf,%lf", &kf.target[0], &kf.target[1], &kf.target[2]);
                    }
                }

                std::string fov = extractTagValue(obj, "\"fov\"");
                if (!fov.empty()) kf.fov = parseDouble(fov, 50.0);

                data.cameraKeyframes.push_back(kf);
                objStart = objEnd + 1;
            }
        }
    }

    // Parse light keyframes array
    size_t lightArrayPos = content.find("\"lightKeyframes\"");
    if (lightArrayPos != std::string::npos) {
        size_t arrStart = content.find('[', lightArrayPos);
        size_t arrEnd = content.find(']', arrStart);
        if (arrStart != std::string::npos && arrEnd != std::string::npos) {
            std::string arrContent = content.substr(arrStart + 1, arrEnd - arrStart - 1);
            size_t objStart = 0;
            while ((objStart = arrContent.find('{', objStart)) != std::string::npos) {
                size_t objEnd = arrContent.find('}', objStart);
                if (objEnd == std::string::npos) break;
                std::string obj = arrContent.substr(objStart, objEnd - objStart + 1);

                LightKeyframe lkf;
                std::string t = extractTagValue(obj, "\"time\"");
                if (!t.empty()) lkf.time = parseDouble(t);

                size_t posArr = obj.find("\"position\"");
                if (posArr != std::string::npos) {
                    size_t pStart = obj.find('[', posArr);
                    size_t pEnd = obj.find(']', pStart);
                    if (pStart != std::string::npos && pEnd != std::string::npos) {
                        std::string pv = obj.substr(pStart + 1, pEnd - pStart - 1);
                        sscanf(pv.c_str(), "%lf,%lf,%lf", &lkf.position[0], &lkf.position[1], &lkf.position[2]);
                    }
                }

                size_t colArr = obj.find("\"color\"");
                if (colArr != std::string::npos) {
                    size_t cStart = obj.find('[', colArr);
                    size_t cEnd = obj.find(']', cStart);
                    if (cStart != std::string::npos && cEnd != std::string::npos) {
                        std::string cv = obj.substr(cStart + 1, cEnd - cStart - 1);
                        sscanf(cv.c_str(), "%lf,%lf,%lf", &lkf.color[0], &lkf.color[1], &lkf.color[2]);
                    }
                }

                std::string intens = extractTagValue(obj, "\"intensity\"");
                if (!intens.empty()) lkf.intensity = parseDouble(intens, 1.0);

                lkf.lightType = extractTagValue(obj, "\"lightType\"");

                data.lightKeyframes.push_back(lkf);
                objStart = objEnd + 1;
            }
        }
    }

    return data;
}

CameraData CameraDataImporter::importFromFBX(const std::string& path) {
    CameraData data;
    if (!parseFBXCamera(path, data)) {
        data.sourceApp = "FBX (parse failed)";
    }
    return data;
}

bool CameraDataImporter::parseFBXCamera(const std::string& path, CameraData& data) {
    std::string content = readFile(path);
    if (content.empty()) return false;

    data.sourceApp = "FBX";

    // Detect FBX version
    std::string ver = extractTagValue(content, "FBXHeaderExtension:FBXVersion:");
    if (ver.empty()) ver = extractTagValue(content, "FBXVersion:");

    // FBX is a binary or ASCII format
    // Binary: starts with "Kaydara FBX Binary"
    if (content.size() >= 21 && content.substr(0, 21) == "Kaydara FBX Binary  \0") {
        // Parse binary FBX
        const char* data_ptr = content.data();
        size_t data_size = content.size();

        // Find camera nodes in binary FBX by searching for ASCII strings within
        std::string searchStr = "Camera";
        size_t pos = 0;
        while (pos < data_size) {
            size_t found = content.find(searchStr, pos);
            if (found == std::string::npos) break;

            // Check if this is inside a Camera node
            // Look backwards for the node start
            CameraKeyframe kf;
            // Read some nearby float values as position (heuristic)
            if (found + 32 < data_size) {
                // FBX stores floats in little-endian IEEE754
                const float* fptr = reinterpret_cast<const float*>(data_ptr + found);
                // Use nearby data as position hint
                if (found + 8 < data_size) {
                    // Just set default camera for now
                    kf.position[0] = 0;
                    kf.position[1] = 0;
                    kf.position[2] = 500;
                }
            }

            // Only add one camera from binary scan
            if (data.cameraKeyframes.empty()) {
                data.cameraKeyframes.push_back(kf);
            }
            break;
        }
        return true;
    }

    // ASCII FBX format
    // Look for Camera nodes
    size_t camPos = 0;
    while ((camPos = content.find("Camera", camPos)) != std::string::npos) {
        // Make sure this is a node name, not a property reference
        if (camPos == 0 || (content[camPos - 1] == '\n' || content[camPos - 1] == ' ')) {
            size_t lineEnd = content.find('\n', camPos);
            std::string line = content.substr(camPos, lineEnd - camPos);

            // Check if it's a Camera node definition (not "CameraInfo" etc)
            if (line.find("Camera{") != std::string::npos || line.find("Camera \"") != std::string::npos) {
                CameraKeyframe kf;

                // Look for Properties70 section which contains position, rotation
                size_t propsStart = content.find("Properties70:", camPos);
                size_t nodeEnd = content.find("\n} ", propsStart);
                if (nodeEnd == std::string::npos) {
                    // Find the matching closing brace
                    int depth = 1;
                    nodeEnd = camPos;
                    while (nodeEnd < content.size() && depth > 0) {
                        if (content[nodeEnd] == '{') depth++;
                        if (content[nodeEnd] == '}') depth--;
                        nodeEnd++;
                    }
                }

                if (propsStart < nodeEnd && propsStart != std::string::npos) {
                    std::string propsSection = content.substr(propsStart, nodeEnd - propsStart);

                    // Find "Lcl Translation" property - contains X,Y,Z
                    size_t transPos = propsSection.find("Lcl Translation");
                    if (transPos != std::string::npos) {
                        // Values are on the next line(s)
                        size_t valLine = propsSection.find('\n', transPos);
                        if (valLine != std::string::npos) {
                            valLine++;
                            std::string valStr = propsSection.substr(valLine, 200);
                            sscanf(valStr.c_str(), " %lf %lf %lf", &kf.position[0], &kf.position[1], &kf.position[2]);
                        }
                    }

                    // Find "Lcl Rotation"
                    size_t rotPos = propsSection.find("Lcl Rotation");
                    if (rotPos != std::string::npos) {
                        // We can use rotation to compute target later
                    }

                    // Find "FocalLength"
                    size_t focalPos = propsSection.find("FocalLength");
                    if (focalPos != std::string::npos) {
                        size_t valLine = propsSection.find('\n', focalPos);
                        if (valLine != std::string::npos) {
                            valLine++;
                            std::string valStr = propsSection.substr(valLine, 100);
                            double focal = 0;
                            if (sscanf(valStr.c_str(), " %lf", &focal) == 1) {
                                // Convert focal length to FOV: fov = 2 * atan(apertureWidth/2 / focalLength) * 180/pi
                                double fov = 2.0 * std::atan(36.0 / (2.0 * focal)) * 180.0 / M_PI;
                                kf.fov = fov;
                            }
                        }
                    }
                }

                if (data.cameraKeyframes.empty() || std::abs(kf.time - data.cameraKeyframes.back().time) > 0.001) {
                    data.cameraKeyframes.push_back(kf);
                }
            }
        }
        camPos++;
    }

    // Also extract light data from FBX
    size_t lightPos = 0;
    while ((lightPos = content.find("Light", lightPos)) != std::string::npos) {
        if (lightPos == 0 || (content[lightPos - 1] == '\n' || content[lightPos - 1] == ' ')) {
            size_t lineEnd = content.find('\n', lightPos);
            std::string line = content.substr(lightPos, lineEnd - lightPos);

            if (line.find("Light{") != std::string::npos || line.find("Light \"") != std::string::npos) {
                LightKeyframe lkf;
                size_t propsStart = content.find("Properties70:", lightPos);
                size_t nodeEnd = content.find("\n} ", propsStart);
                if (nodeEnd == std::string::npos) nodeEnd = lightPos + 500;

                if (propsStart < nodeEnd && propsStart != std::string::npos) {
                    std::string propsSection = content.substr(propsStart, nodeEnd - propsStart);

                    size_t transPos = propsSection.find("Lcl Translation");
                    if (transPos != std::string::npos) {
                        size_t valLine = propsSection.find('\n', transPos);
                        if (valLine != std::string::npos) {
                            valLine++;
                            std::string valStr = propsSection.substr(valLine, 200);
                            sscanf(valStr.c_str(), " %lf %lf %lf", &lkf.position[0], &lkf.position[1], &lkf.position[2]);
                        }
                    }

                    size_t colorPos = propsSection.find("Color");
                    if (colorPos != std::string::npos) {
                        size_t valLine = propsSection.find('\n', colorPos);
                        if (valLine != std::string::npos) {
                            valLine++;
                            std::string valStr = propsSection.substr(valLine, 200);
                            sscanf(valStr.c_str(), " %lf %lf %lf", &lkf.color[0], &lkf.color[1], &lkf.color[2]);
                        }
                    }

                    size_t intensityPos = propsSection.find("Intensity");
                    if (intensityPos != std::string::npos) {
                        size_t valLine = propsSection.find('\n', intensityPos);
                        if (valLine != std::string::npos) {
                            valLine++;
                            std::string valStr = propsSection.substr(valLine, 100);
                            double intens = 100;
                            if (sscanf(valStr.c_str(), " %lf", &intens) == 1) {
                                lkf.intensity = intens / 100.0;
                            }
                        }
                    }
                }

                data.lightKeyframes.push_back(lkf);
            }
        }
        lightPos++;
    }

    return !data.cameraKeyframes.empty();
}

CameraData CameraDataImporter::importFromDAE(const std::string& path) {
    CameraData data;
    if (!parseDAECamera(path, data)) {
        data.sourceApp = "COLLADA (parse failed)";
    }
    return data;
}

bool CameraDataImporter::parseDAECamera(const std::string& path, CameraData& data) {
    std::string content = readFile(path);
    if (content.empty()) return false;

    data.sourceApp = "COLLADA";

    // Verify it's a COLLADA file
    if (content.find("<COLLADA") == std::string::npos) return false;

    // Find camera definitions: <camera> ... <optics> ... <perspective> ... < fov > </ fov >
    size_t camPos = 0;
    while ((camPos = content.find("<camera", camPos)) != std::string::npos) {
        size_t camEnd = content.find("</camera>", camPos);
        if (camEnd == std::string::npos) { camPos++; continue; }

        std::string camSection = content.substr(camPos, camEnd - camPos);

        CameraKeyframe kf;

        // Get camera id/name
        size_t idPos = camSection.find("id=\"");
        if (idPos != std::string::npos) {
            size_t idStart = idPos + 4;
            size_t idEnd = camSection.find("\"", idStart);
        }

        // Get FOV (xfov or yfov)
        size_t xfovPos = camSection.find("<xfov>");
        if (xfovPos != std::string::npos) {
            size_t end = camSection.find("</xfov>", xfovPos);
            std::string fovStr = camSection.substr(xfovPos + 6, end - xfovPos - 6);
            kf.fov = parseDouble(fovStr, 50.0);
        }

        size_t yfovPos = camSection.find("<yfov>");
        if (yfovPos != std::string::npos) {
            size_t end = camSection.find("</yfov>", yfovPos);
            std::string fovStr = camSection.substr(yfovPos + 6, end - yfovPos - 6);
            if (xfovPos == std::string::npos) {
                kf.fov = parseDouble(fovStr, 50.0);
            }
        }

        data.cameraKeyframes.push_back(kf);
        camPos = camEnd + 10;
    }

    // Now find camera instances in the visual scene to get positions
    // Look for <instance_camera> and associated node transforms
    size_t nodePos = 0;
    int camIdx = 0;
    while ((nodePos = content.find("<node", nodePos)) != std::string::npos) {
        size_t nodeEnd = content.find("</node>", nodePos);
        if (nodeEnd == std::string::npos) { nodePos++; continue; }

        std::string nodeSection = content.substr(nodePos, nodeEnd - nodePos);

        // Check if this node contains an instance_camera
        if (nodeSection.find("<instance_camera") != std::string::npos) {
            // Extract transform matrix
            size_t matPos = nodeSection.find("<matrix sid=\"transform\">");
            if (matPos != std::string::npos) {
                size_t matEnd = nodeSection.find("</matrix>", matPos);
                std::string matStr = nodeSection.substr(matPos + 25, matEnd - matPos - 25);

                // Parse 4x4 matrix (16 floats, column-major)
                double matrix[16] = {};
                int count = 0;
                std::istringstream iss(matStr);
                while (iss >> matrix[count] && count < 16) count++;

                if (count == 16 && camIdx < static_cast<int>(data.cameraKeyframes.size())) {
                    // Extract position from translation column (column 3, rows 0-2)
                    data.cameraKeyframes[camIdx].position[0] = matrix[12];
                    data.cameraKeyframes[camIdx].position[1] = matrix[13];
                    data.cameraKeyframes[camIdx].position[2] = matrix[14];
                    camIdx++;
                }
            }

            // Also check for <translate> transform
            size_t transPos = nodeSection.find("<translate");
            if (transPos != std::string::npos && camIdx <= static_cast<int>(data.cameraKeyframes.size())) {
                size_t transEnd = nodeSection.find("</translate>", transPos);
                size_t tagEnd = nodeSection.find('>', transPos);
                std::string transStr = nodeSection.substr(tagEnd + 1, transEnd - tagEnd - 1);
                if (camIdx > 0 && camIdx <= static_cast<int>(data.cameraKeyframes.size())) {
                    sscanf(transStr.c_str(), "%lf %lf %lf",
                            &data.cameraKeyframes[camIdx - 1].position[0],
                            &data.cameraKeyframes[camIdx - 1].position[1],
                            &data.cameraKeyframes[camIdx - 1].position[2]);
                }
            }
        }

        // Check for lights
        if (nodeSection.find("<instance_light") != std::string::npos) {
            LightKeyframe lkf;
            size_t matPos = nodeSection.find("<matrix sid=\"transform\">");
            if (matPos != std::string::npos) {
                size_t matEnd = nodeSection.find("</matrix>", matPos);
                std::string matStr = nodeSection.substr(matPos + 25, matEnd - matPos - 25);
                double matrix[16] = {};
                int count = 0;
                std::istringstream iss(matStr);
                while (iss >> matrix[count] && count < 16) count++;
                if (count == 16) {
                    lkf.position[0] = matrix[12];
                    lkf.position[1] = matrix[13];
                    lkf.position[2] = matrix[14];
                }
            }

            size_t transPos = nodeSection.find("<translate");
            if (transPos != std::string::npos) {
                size_t transEnd = nodeSection.find("</translate>", transPos);
                size_t tagEnd = nodeSection.find('>', transPos);
                std::string transStr = nodeSection.substr(tagEnd + 1, transEnd - tagEnd - 1);
                sscanf(transStr.c_str(), "%lf %lf %lf", &lkf.position[0], &lkf.position[1], &lkf.position[2]);
            }

            data.lightKeyframes.push_back(lkf);
        }

        nodePos = nodeEnd + 8;
    }

    return true;
}

CameraData CameraDataImporter::importFromABC(const std::string& path) {
    CameraData data;
    if (!parseABCCamera(path, data)) {
        data.sourceApp = "Alembic (parse failed)";
    }
    return data;
}

bool CameraDataImporter::parseABCCamera(const std::string& path, CameraData& data) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;

    data.sourceApp = "Alembic";

    // Alembic is HDF5-based binary format
    // Check for HDF5 magic number: \x89HDF\r\n\x1a\n
    char magic[8];
    if (fread(magic, 1, 8, f) != 8) { fclose(f); return false; }
    if (memcmp(magic, "\x89HDF\r\n\x1a\n", 8) != 0) {
        // Not HDF5, check if it's OgAWA/Alembic text-based
        fseek(f, 0, SEEK_SET);
        // Try looking for Alembic text headers
        char header[64] = {};
        size_t read = fread(header, 1, sizeof(header) - 1, f);
        header[read] = '\0';

        if (std::string(header).find("Alembic") == std::string::npos) {
            fclose(f);
            return false;
        }
    }

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Alembic files can contain camera data in HDF5 dataset structures
    // For a real parser we'd need HDF5 library, but we can do heuristic scanning
    // Search for camera-related strings within the binary
    std::vector<char> buffer(fileSize);
    size_t totalRead = fread(buffer.data(), 1, fileSize, f);
    std::string content(buffer.data(), totalRead);
    fclose(f);

    // Look for camera property names stored as strings in the HDF5 datasets
    size_t camPos = content.find("camera");
    if (camPos == std::string::npos) camPos = content.find("Camera");
    if (camPos == std::string::npos) camPos = content.find("OCAMERA");

    if (camPos != std::string::npos) {
        CameraKeyframe kf;
        // Try to find focal length nearby
        size_t focalPos = content.find("focalLength", camPos);
        if (focalPos != std::string::npos && focalPos < camPos + 1024) {
            // Read nearby bytes as potential float
            if (focalPos + 8 < content.size()) {
                float focal;
                memcpy(&focal, &content[focalPos + 11], sizeof(float));
                if (focal > 0 && focal < 10000) {
                    kf.fov = 2.0 * std::atan(36.0 / (2.0 * focal)) * 180.0 / M_PI;
                }
            }
        }

        // Try to find position data (camera xform)
        size_t posPos = content.find("xform", camPos);
        if (posPos == std::string::npos) posPos = content.find("position", camPos);

        data.cameraKeyframes.push_back(kf);
    }

    return !data.cameraKeyframes.empty();
}

bool CameraDataImporter::exportToAE(const CameraData& data, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "{\n";
    file << "  \"sourceApp\": \"" << data.sourceApp << "\",\n";
    file << "  \"frameRate\": " << data.frameRate << ",\n";

    // Camera keyframes
    file << "  \"cameraKeyframes\": [\n";
    for (size_t i = 0; i < data.cameraKeyframes.size(); ++i) {
        const auto& kf = data.cameraKeyframes[i];
        file << "    {\n";
        file << "      \"time\": " << kf.time << ",\n";
        file << "      \"position\": [" << kf.position[0] << ", " << kf.position[1] << ", " << kf.position[2] << "],\n";
        file << "      \"target\": [" << kf.target[0] << ", " << kf.target[1] << ", " << kf.target[2] << "],\n";
        file << "      \"up\": [" << kf.up[0] << ", " << kf.up[1] << ", " << kf.up[2] << "],\n";
        file << "      \"fov\": " << kf.fov << "\n";
        file << "    }";
        if (i < data.cameraKeyframes.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ],\n";

    // Light keyframes
    file << "  \"lightKeyframes\": [\n";
    for (size_t i = 0; i < data.lightKeyframes.size(); ++i) {
        const auto& lkf = data.lightKeyframes[i];
        file << "    {\n";
        file << "      \"time\": " << lkf.time << ",\n";
        file << "      \"position\": [" << lkf.position[0] << ", " << lkf.position[1] << ", " << lkf.position[2] << "],\n";
        file << "      \"target\": [" << lkf.target[0] << ", " << lkf.target[1] << ", " << lkf.target[2] << "],\n";
        file << "      \"color\": [" << lkf.color[0] << ", " << lkf.color[1] << ", " << lkf.color[2] << "],\n";
        file << "      \"intensity\": " << lkf.intensity << ",\n";
        file << "      \"lightType\": \"" << lkf.lightType << "\"\n";
        file << "    }";
        if (i < data.lightKeyframes.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ]\n";
    file << "}\n";

    file.close();
    return true;
}

} // namespace FreeEffect
