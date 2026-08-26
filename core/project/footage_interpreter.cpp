#include "footage_interpreter.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace FreeEffect {

std::string FootageInterpreter::toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

std::string FootageInterpreter::getExtension(const std::string& path) {
    auto dot = path.rfind('.');
    if (dot == std::string::npos) return "";
    return toLower(path.substr(dot + 1));
}

void FootageInterpreter::setInterpretation(const std::string& path, const FootageInterpretation& interp) {
    FootageInterpretation i = interp;
    i.path = path;
    m_interpretations[path] = i;
}

FootageInterpretation FootageInterpreter::getInterpretation(const std::string& path) const {
    auto it = m_interpretations.find(path);
    if (it != m_interpretations.end()) return it->second;
    return m_defaultInterp;
}

bool FootageInterpreter::hasInterpretation(const std::string& path) const {
    return m_interpretations.count(path) > 0;
}

void FootageInterpreter::removeInterpretation(const std::string& path) {
    m_interpretations.erase(path);
}

FootageInterpretation FootageInterpreter::autoDetect(const std::string& path) const {
    FootageInterpretation interp = m_defaultInterp;
    interp.path = path;

    std::string ext = getExtension(path);

    if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "tiff" || ext == "tif" || ext == "bmp" || ext == "psd") {
        interp.frameRate = 30.0;
        interp.alphaMode = (ext == "png" || ext == "psd") ? 0 : 2;
        interp.interpretAlpha = (ext == "png" || ext == "psd");
    } else if (ext == "exr" || ext == "hdr") {
        interp.frameRate = 30.0;
        interp.alphaMode = 1;
        interp.colorDepth = 2;
        interp.interpretAlpha = true;
    } else if (ext == "mp4" || ext == "mov" || ext == "avi" || ext == "mkv" || ext == "webm") {
        interp.frameRate = 29.97;
        interp.alphaMode = 0;
        interp.fieldOrder = 0;
        interp.interpretAlpha = true;
    } else if (ext == "wav" || ext == "mp3" || ext == "aac" || ext == "flac" || ext == "ogg") {
        interp.frameRate = 30.0;
        interp.interpretAlpha = false;
        interp.alphaMode = 2;
    } else if (ext == "tif" || ext == "tiff") {
        interp.frameRate = 30.0;
        interp.alphaMode = 0;
        interp.colorDepth = 1;
        interp.interpretAlpha = true;
    } else if (ext == "dpx") {
        interp.frameRate = 24.0;
        interp.alphaMode = 0;
        interp.colorDepth = 2;
        interp.interpretAlpha = true;
    } else if (ext == "cin") {
        interp.frameRate = 24.0;
        interp.alphaMode = 2;
        interp.colorDepth = 2;
        interp.interpretAlpha = false;
    }

    return interp;
}

bool FootageInterpreter::saveInterpretations(const std::string& projectPath) {
    std::string interpPath = projectPath + ".interp";
    std::ofstream file(interpPath);
    if (!file.is_open()) return false;

    file << "FEINTv1\n";
    file << m_defaultInterp.frameRate << "\n";
    file << m_defaultInterp.alphaMode << "\n";
    file << m_defaultInterp.pixelAspect << "\n";
    file << m_defaultInterp.fieldOrder << "\n";
    file << (m_defaultInterp.loopFootage ? 1 : 0) << "\n";
    file << m_defaultInterp.loopCount << "\n";
    file << m_defaultInterp.proxyScale << "\n";
    file << m_defaultInterp.colorDepth << "\n";
    file << m_defaultInterp.colorProfile << "\n";
    file << (m_defaultInterp.interpretAlpha ? 1 : 0) << "\n";
    file << m_defaultInterp.startTime << "\n";

    file << m_interpretations.size() << "\n";
    for (const auto& [path, interp] : m_interpretations) {
        file << path << "\n";
        file << interp.frameRate << "\n";
        file << interp.startTime << "\n";
        file << interp.alphaMode << "\n";
        file << interp.premultiplyColor.r << " " << interp.premultiplyColor.g << " "
             << interp.premultiplyColor.b << " " << interp.premultiplyColor.a << "\n";
        file << interp.pixelAspect << "\n";
        file << interp.fieldOrder << "\n";
        file << (interp.loopFootage ? 1 : 0) << "\n";
        file << interp.loopCount << "\n";
        file << interp.proxyScale << "\n";
        file << interp.colorDepth << "\n";
        file << interp.colorProfile << "\n";
        file << (interp.interpretAlpha ? 1 : 0) << "\n";
    }
    return file.good();
}

bool FootageInterpreter::loadInterpretations(const std::string& projectPath) {
    std::string interpPath = projectPath + ".interp";
    std::ifstream file(interpPath);
    if (!file.is_open()) return false;

    std::string header;
    std::getline(file, header);
    if (header != "FEINTv1") return false;

    file >> m_defaultInterp.frameRate;
    file >> m_defaultInterp.alphaMode;
    file >> m_defaultInterp.pixelAspect;
    file >> m_defaultInterp.fieldOrder;
    int loopFlag = 0;
    file >> loopFlag;
    m_defaultInterp.loopFootage = (loopFlag != 0);
    file >> m_defaultInterp.loopCount;
    file >> m_defaultInterp.proxyScale;
    file >> m_defaultInterp.colorDepth;
    file.ignore();
    std::getline(file, m_defaultInterp.colorProfile);
    int alphaFlag = 0;
    file >> alphaFlag;
    m_defaultInterp.interpretAlpha = (alphaFlag != 0);
    file >> m_defaultInterp.startTime;
    file.ignore();

    size_t count = 0;
    file >> count;
    file.ignore();

    m_interpretations.clear();
    for (size_t i = 0; i < count; ++i) {
        FootageInterpretation interp;
        std::getline(file, interp.path);
        file >> interp.frameRate;
        file >> interp.startTime;
        file >> interp.alphaMode;
        file >> interp.premultiplyColor.r >> interp.premultiplyColor.g
             >> interp.premultiplyColor.b >> interp.premultiplyColor.a;
        file >> interp.pixelAspect;
        file >> interp.fieldOrder;
        int lf = 0;
        file >> lf;
        interp.loopFootage = (lf != 0);
        file >> interp.loopCount;
        file >> interp.proxyScale;
        file >> interp.colorDepth;
        file.ignore();
        std::getline(file, interp.colorProfile);
        int af = 0;
        file >> af;
        interp.interpretAlpha = (af != 0);
        file.ignore();

        m_interpretations[interp.path] = interp;
    }
    return true;
}

} // namespace FreeEffect
