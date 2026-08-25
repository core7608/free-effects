#pragma once

#include "command.h"
#include "../timeline/layer.h"
#include <functional>

namespace FreeEffect {

class SetKeyframeCommand : public Command {
public:
    SetKeyframeCommand(Layer* layer, const std::string& propertyName, 
                       double time, double value, 
                       InterpolationType interp = InterpolationType::Linear);
    
    void execute() override;
    void undo() override;
    
    std::string getDescription() const override;

private:
    Layer* m_layer;
    std::string m_propertyName;
    double m_time;
    double m_value;
    InterpolationType m_interpolation;
    
    PropertyTrack* getPropertyTrack() const;
    std::optional<Keyframe> m_previousKeyframe;
    bool m_hadPreviousKeyframe = false;
};

} // namespace FreeEffect
