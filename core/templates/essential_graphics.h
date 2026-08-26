#pragma once
#include "../timeline/types.h"
#include <string>
#include <vector>

namespace FreeEffect {

struct EssentialProperty {
    std::string name;
    std::string layerName;
    std::string propertyName;
    double min = 0, max = 100;
    std::string tooltip;
};

class EssentialGraphics {
public:
    void addProperty(const EssentialProperty& prop);
    void removeProperty(int index);
    const std::vector<EssentialProperty>& getProperties() const;
    
    bool exportMotionGraphicTemplate(const std::string& filePath) const;
    bool importMotionGraphicTemplate(const std::string& filePath);
    
    std::string getCompName() const { return m_compName; }
    void setCompName(const std::string& name) { m_compName = name; }

private:
    std::string m_compName;
    std::vector<EssentialProperty> m_properties;
};

} // namespace FreeEffect
