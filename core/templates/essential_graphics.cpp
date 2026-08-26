#include "essential_graphics.h"
#include <fstream>
#include <sstream>

namespace FreeEffect {

void EssentialGraphics::addProperty(const EssentialProperty& prop) {
    m_properties.push_back(prop);
}

void EssentialGraphics::removeProperty(int index) {
    if (index >= 0 && index < static_cast<int>(m_properties.size())) {
        m_properties.erase(m_properties.begin() + index);
    }
}

const std::vector<EssentialProperty>& EssentialGraphics::getProperties() const {
    return m_properties;
}

bool EssentialGraphics::exportMotionGraphicTemplate(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    file << "FreeEffect Motion Graphic Template\n";
    file << "==================================\n";
    file << "Composition: " << m_compName << "\n";
    file << "PropertyCount: " << m_properties.size() << "\n\n";

    for (size_t i = 0; i < m_properties.size(); ++i) {
        const auto& prop = m_properties[i];
        file << "Property[" << i << "]\n";
        file << "  Name: " << prop.name << "\n";
        file << "  LayerName: " << prop.layerName << "\n";
        file << "  PropertyName: " << prop.propertyName << "\n";
        file << "  Min: " << prop.min << "\n";
        file << "  Max: " << prop.max << "\n";
        file << "  Tooltip: " << prop.tooltip << "\n\n";
    }

    return true;
}

bool EssentialGraphics::importMotionGraphicTemplate(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    m_properties.clear();
    std::string line;

    while (std::getline(file, line)) {
        if (line.find("Composition: ") == 0) {
            m_compName = line.substr(13);
        } else if (line.find("  Name: ") == 0) {
            EssentialProperty prop;
            prop.name = line.substr(8);
            std::getline(file, line);
            if (line.find("  LayerName: ") == 0) prop.layerName = line.substr(13);
            std::getline(file, line);
            if (line.find("  PropertyName: ") == 0) prop.propertyName = line.substr(16);
            std::getline(file, line);
            if (line.find("  Min: ") == 0) prop.min = std::stod(line.substr(7));
            std::getline(file, line);
            if (line.find("  Max: ") == 0) prop.max = std::stod(line.substr(7));
            std::getline(file, line);
            if (line.find("  Tooltip: ") == 0) prop.tooltip = line.substr(11);
            m_properties.push_back(prop);
        }
    }

    return true;
}

} // namespace FreeEffect
