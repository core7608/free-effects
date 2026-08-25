#pragma once

#include "command.h"
#include "../timeline/composition.h"

namespace FreeEffect {

class MoveLayerCommand : public Command {
public:
    MoveLayerCommand(Composition* comp, int fromIndex, int toIndex);
    
    void execute() override;
    void undo() override;
    
    std::string getDescription() const override;

private:
    Composition* m_composition;
    int m_fromIndex;
    int m_toIndex;
};

} // namespace FreeEffect
