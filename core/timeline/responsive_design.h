#pragma once
#include "property_track.h"
#include <vector>

namespace FreeEffect {

struct ResponsiveRegion {
    double startTime = 0;   // Intro region end
    double endTime = 0;     // Outro region start
    bool isIntro = false;
    bool isOutro = false;
};

struct PositionPin {
    double time = 0;
    double percentage = 0;  // 0-100, percentage of layer duration
    std::string description;
};

class ResponsiveDesign {
public:
    void setIntroRegion(double duration);
    void setOutroRegion(double duration);
    void addPositionPin(double time, double percentage, const std::string& desc = "");
    void removePositionPin(int index);
    
    void setTimeStretch(double factor);  // Stretches only the middle (non-intro/outro)
    
    const std::vector<ResponsiveRegion>& getRegions() const { return m_regions; }
    const std::vector<PositionPin>& getPositionPins() const { return m_pins; }

private:
    std::vector<ResponsiveRegion> m_regions;
    std::vector<PositionPin> m_pins;
};

} // namespace FreeEffect
