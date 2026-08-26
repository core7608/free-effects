#pragma once

#include "command.h"
#include "../timeline/layer.h"
#include "../timeline/composition.h"
#include "../timeline/keyframe.h"
#include <vector>
#include <utility>

namespace FreeEffect {

class TimeReverseCommand : public Command {
public:
    TimeReverseCommand(Composition* comp, int layerIndex);

    void execute() override;
    void undo() override;
    std::string getDescription() const override;

private:
    Composition* m_comp;
    int m_layerIndex;

    struct PropertyBackup {
        PropertyTrack* track = nullptr;
        KeyframeList originalKeyframes;
    };
    std::vector<PropertyBackup> m_backups;
    bool m_executed = false;
};

} // namespace FreeEffect
