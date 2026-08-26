#pragma once
#include <string>
#include <unordered_map>
#include "../timeline/types.h"

namespace FreeEffect {

struct FootageInterpretation {
    std::string path;
    double frameRate = 30.0;
    double startTime = 0;
    int alphaMode = 0; // 0=Straight, 1=Premultiplied, 2=Ignore, 3=Straight Unmatted
    Color premultiplyColor{0, 0, 0, 1.0};
    int pixelAspect = 1;
    int fieldOrder = 0; // 0=None, 1=Upper First, 2=Lower First
    bool loopFootage = false;
    int loopCount = 1;
    int proxyScale = 100;
    int colorDepth = 0; // 0=8bpc, 1=16bpc, 2=32bpc
    std::string colorProfile;
    bool interpretAlpha = true;
};

class FootageInterpreter {
public:
    void setInterpretation(const std::string& path, const FootageInterpretation& interp);
    FootageInterpretation getInterpretation(const std::string& path) const;
    bool hasInterpretation(const std::string& path) const;
    void removeInterpretation(const std::string& path);

    void setDefaultInterpretation(const FootageInterpretation& interp) { m_defaultInterp = interp; }
    FootageInterpretation getDefaultInterpretation() const { return m_defaultInterp; }

    FootageInterpretation autoDetect(const std::string& path) const;

    bool saveInterpretations(const std::string& projectPath);
    bool loadInterpretations(const std::string& projectPath);

    std::unordered_map<std::string, FootageInterpretation>& getAllInterpretations() { return m_interpretations; }
    const std::unordered_map<std::string, FootageInterpretation>& getAllInterpretations() const { return m_interpretations; }

private:
    std::unordered_map<std::string, FootageInterpretation> m_interpretations;
    FootageInterpretation m_defaultInterp;

    static std::string toLower(const std::string& s);
    static std::string getExtension(const std::string& path);
};

} // namespace FreeEffect
