#pragma once

#include "command.h"
#include "../timeline/composition.h"
#include "../timeline/layer.h"
#include <vector>
#include <memory>

namespace FreeEffect {

class PrecomposeCommand : public Command {
public:
    PrecomposeCommand(Composition* comp, const std::vector<int>& layerIndices, const std::string& name);

    void execute() override;
    void undo() override;
    std::string getDescription() const override;

private:
    Composition* m_comp;
    std::vector<int> m_layerIndices;
    std::string m_name;
    std::vector<LayerPtr> m_originalLayers;
    std::vector<double> m_originalStartTimes;
    std::vector<double> m_originalDurations;
    LayerPtr m_precompLayer;
    std::shared_ptr<Composition> m_nestedComp;
    int m_insertIndex = 0;
};

} // namespace FreeEffect
