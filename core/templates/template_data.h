#pragma once

#include <map>
#include <string>
#include <vector>

namespace FreeEffect {

struct TemplateEffect {
    std::string effectName;
    std::map<std::string, double> parameters;
};

struct TemplateLayer {
    std::string name;
    std::string type;
    double startTime = 0.0;
    double duration = 0.0;
    double positionX = 960.0;
    double positionY = 540.0;
    double scaleX = 100.0;
    double scaleY = 100.0;
    double rotation = 0.0;
    double opacity = 100.0;
    std::string text;
    std::string fontFamily;
    double fontSize = 48.0;
    double textColorR = 1.0;
    double textColorG = 1.0;
    double textColorB = 1.0;
    double textColorA = 1.0;
    std::string sourcePath;
    double colorR = 1.0;
    double colorG = 1.0;
    double colorB = 1.0;
    double colorA = 1.0;
    std::string blendMode;
    std::vector<TemplateEffect> effects;
    std::map<std::string, std::vector<std::map<std::string, double>>> keyframes;
};

struct TemplateData {
    std::string name;
    std::string description;
    std::string category;
    std::string thumbnail;
    double duration = 10.0;
    int width = 1920;
    int height = 1080;
    double fps = 30.0;
    double backgroundR = 0.0;
    double backgroundG = 0.0;
    double backgroundB = 0.0;
    double backgroundA = 1.0;
    std::vector<TemplateLayer> layers;
};

} // namespace FreeEffect
