#pragma once

#include "command.h"
#include "../timeline/composition.h"
#include "../timeline/layer.h"

namespace FreeEffect {

class AddLayerCommand : public Command {
public:
    AddLayerCommand(Composition* comp, const std::string& name, LayerType type, int insertIndex = -1);
    
    void execute() override;
    void undo() override;
    
    std::string getDescription() const override;
    
    LayerPtr getLayer() const { return m_layer; }

private:
    Composition* m_composition;
    std::string m_name;
    LayerType m_type;
    int m_insertIndex;
    LayerPtr m_layer;
    int m_executedIndex = -1;
};

} // namespace FreeEffect
