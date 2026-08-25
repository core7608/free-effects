#pragma once

#include "command.h"
#include "../timeline/layer.h"
#include <functional>

namespace FreeEffect {

class RemoveKeyframeCommand : public Command {
public:
    RemoveKeyframeCommand(Layer* layer, const std::string& propertyName, double time);
    
    void execute() override;
    void undo() override;
    
    std::string getDescription() const override;

private:
    Layer* m_layer;
    std::string m_propertyName;
    double m_time;
    
    PropertyTrack* getPropertyTrack() const;
    std::optional<Keyframe> m_removedKeyframe;
};

} // namespace FreeEffect
