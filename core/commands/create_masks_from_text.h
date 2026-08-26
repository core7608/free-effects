#pragma once

#include "command.h"
#include "../timeline/composition.h"
#include "../timeline/layer.h"
#include "../timeline/shape_layer.h"
#include <vector>

namespace FreeEffect {

class CreateMasksFromTextCommand : public Command {
public:
    CreateMasksFromTextCommand(Composition* comp, int layerIndex);

    void execute() override;
    void undo() override;
    std::string getDescription() const override;

private:
    Composition* m_comp;
    int m_layerIndex;
    LayerPtr m_targetLayer;
    std::vector<ShapePath> m_createdMasks;
    bool m_executed = false;

    std::vector<ShapePath> generateTextMaskPaths() const;
};

} // namespace FreeEffect
