#pragma once

#include "command.h"
#include "../timeline/layer.h"
#include "../timeline/composition.h"

namespace FreeEffect {

class SplitLayerCommand : public Command {
public:
    SplitLayerCommand(Composition* comp, int layerIndex, double time);

    void execute() override;
    void undo() override;
    std::string getDescription() const override;

private:
    Composition* m_comp;
    int m_layerIndex;
    double m_splitTime;
    LayerPtr m_newLayer;
    std::string m_originalName;
    double m_originalStartTime = 0.0;
    double m_originalDuration = 0.0;
    LayerType m_layerType = LayerType::Solid;
};

} // namespace FreeEffect
