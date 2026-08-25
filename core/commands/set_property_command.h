#pragma once

#include "command.h"
#include "../timeline/layer.h"
#include <string>

namespace FreeEffect {

class SetPropertyCommand : public Command {
public:
    SetPropertyCommand(Layer* layer, const std::string& propertyName, double value);
    
    void execute() override;
    void undo() override;
    
    std::string getDescription() const override;

private:
    Layer* m_layer;
    std::string m_propertyName;
    double m_value;
    double m_previousValue = 0.0;
    
    PropertyTrack* getPropertyTrack() const;
};

} // namespace FreeEffect
