#pragma once

#include "command.h"
#include "../timeline/layer.h"
#include "../timeline/composition.h"

namespace FreeEffect {

class FreezeFrameCommand : public Command {
public:
    FreezeFrameCommand(Composition* comp, int layerIndex, double freezeTime);

    void execute() override;
    void undo() override;
    std::string getDescription() const override;

private:
    Composition* m_comp;
    int m_layerIndex;
    double m_freezeTime;
    double m_originalDuration = 0.0;
    double m_originalStartTime = 0.0;
    bool m_originalTimeRemapEnabled = false;
    bool m_executed = false;
};

} // namespace FreeEffect
