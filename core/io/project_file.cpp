#include "project_file.h"
#include <fstream>
#include <sstream>

namespace FreeEffect {

bool ProjectFile::save(const ProjectState& project, const std::string& filePath) {
    // Build JSON manually for MVP (nlohmann/json integration coming later)
    std::ostringstream json;
    json << "{\n";
    json << "  \"freeeffect_version\": \"" << getVersion() << "\",\n";
    json << "  \"project_settings\": {\n";
    json << "    \"frame_rate_base\": " << project.getSettings().frameRateBase << "\n";
    json << "  },\n";
    
    // Assets
    json << "  \"assets\": [\n";
    const auto& assets = project.getAssets();
    for (size_t i = 0; i < assets.size(); ++i) {
        const auto& asset = assets[i];
        json << "    {\n";
        json << "      \"id\": \"" << asset->getId() << "\",\n";
        
        std::string typeStr;
        switch (asset->getType()) {
            case AssetType::Video: typeStr = "video"; break;
            case AssetType::Image: typeStr = "image"; break;
            case AssetType::Audio: typeStr = "audio"; break;
            case AssetType::Composition: typeStr = "composition"; break;
        }
        json << "      \"type\": \"" << typeStr << "\",\n";
        json << "      \"path\": \"" << asset->getPath() << "\",\n";
        json << "      \"name\": \"" << asset->getName() << "\"\n";
        json << "    }";
        if (i < assets.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ],\n";
    
    // Compositions
    json << "  \"compositions\": [\n";
    const auto& comps = project.getCompositions();
    for (size_t i = 0; i < comps.size(); ++i) {
        const auto& comp = comps[i];
        json << "    {\n";
        json << "      \"id\": \"" << comp->getId() << "\",\n";
        json << "      \"name\": \"" << comp->getName() << "\",\n";
        json << "      \"width\": " << comp->getResolution().width << ",\n";
        json << "      \"height\": " << comp->getResolution().height << ",\n";
        json << "      \"frame_rate\": " << comp->getFrameRate().fps << ",\n";
        json << "      \"duration\": " << comp->getDuration() << ",\n";
        json << "      \"layers\": [\n";
        
        const auto& layers = comp->getLayers();
        for (size_t j = 0; j < layers.size(); ++j) {
            const auto& layer = layers[j];
            json << "        {\n";
            json << "          \"id\": \"" << layer->getId() << "\",\n";
            json << "          \"name\": \"" << layer->getName() << "\",\n";
            json << "          \"start_time\": " << layer->getStartTime() << ",\n";
            json << "          \"duration\": " << layer->getDuration() << ",\n";
            json << "          \"transform\": {\n";
            
            auto writeTrack = [&](const std::string& name, const PropertyTrack& track) {
                json << "            \"" << name << "\": {\n";
                json << "              \"keyframes\": [";
                const auto& keyframes = track.getKeyframes();
                for (size_t k = 0; k < keyframes.size(); ++k) {
                    json << "{\"time\": " << keyframes[k].getTime() 
                         << ", \"value\": " << keyframes[k].getValue() << "}";
                    if (k < keyframes.size() - 1) json << ", ";
                }
                json << "]\n";
                json << "            }";
            };
            
            writeTrack("position", layer->getPosition()); json << ",\n";
            writeTrack("scale", layer->getScale()); json << ",\n";
            writeTrack("rotation", layer->getRotation()); json << ",\n";
            writeTrack("opacity", layer->getOpacity()); json << ",\n";
            writeTrack("anchor_point", layer->getAnchorPoint());
            
            json << "\n          }\n";
            json << "        }";
            if (j < layers.size() - 1) json << ",";
            json << "\n";
        }
        
        json << "      ]\n";
        json << "    }";
        if (i < comps.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ]\n";
    json << "}\n";
    
    std::ofstream file(filePath);
    if (!file.is_open()) return false;
    
    file << json.str();
    file.close();
    return true;
}

ProjectLoadResult ProjectFile::load(const std::string& filePath, ProjectState& project) {
    ProjectLoadResult result;
    result.filePath = filePath;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        result.errorMessage = "Could not open file: " + filePath;
        return result;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();
    
    // Basic validation
    if (content.find("\"freeeffect_version\"") == std::string::npos) {
        result.errorMessage = "Invalid project file format";
        return result;
    }
    
    // Parse version
    auto versionPos = content.find("\"freeeffect_version\"");
    if (versionPos != std::string::npos) {
        auto colonPos = content.find(':', versionPos);
        auto quoteStart = content.find('"', colonPos + 1);
        auto quoteEnd = content.find('"', quoteStart + 1);
        // Version validation will be more thorough with nlohmann/json
    }
    
    project.clear();
    project.setFilePath(filePath);
    
    result.success = true;
    return result;
}

bool ProjectFile::saveToCurrentPath(const ProjectState& project) {
    if (project.getFilePath().empty()) return false;
    return save(project, project.getFilePath());
}

} // namespace FreeEffect
