#pragma once

#include "command.h"
#include "../timeline/composition.h"
#include "../timeline/layer.h"

namespace FreeEffect {

class RemoveLayerCommand : public Command {
public:
    RemoveLayerCommand(Composition* comp, const UUID& layerId);
    
    void execute() override;
    void undo() override;
    
    std::string getDescription() const override;

private:
    Composition* m_composition;
    UUID m_layerId;
    LayerPtr m_removedLayer;
    int m_layerIndex = -1;
    
    void restoreLayer();
};

} // namespace FreeEffect
