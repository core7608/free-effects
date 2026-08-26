#pragma once

#include "command.h"
#include "../timeline/composition.h"
#include "../timeline/layer.h"
#include "../timeline/shape_layer.h"
#include <vector>
#include <memory>

namespace FreeEffect {

class CreateShapesFromTextCommand : public Command {
public:
    CreateShapesFromTextCommand(Composition* comp, int layerIndex);

    void execute() override;
    void undo() override;
    std::string getDescription() const override;

private:
    Composition* m_comp;
    int m_layerIndex;
    LayerPtr m_shapeLayer;
    LayerPtr m_originalTextLayer;
    bool m_executed = false;

    std::vector<ShapeGroup> generateTextShapes() const;
    ShapeGroup createCharacterShape(char ch, double x, double y,
                                    double charWidth, double charHeight) const;
};

} // namespace FreeEffect
