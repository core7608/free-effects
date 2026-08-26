#include "time_reverse_command.h"
#include <algorithm>

namespace FreeEffect {

TimeReverseCommand::TimeReverseCommand(Composition* comp, int layerIndex)
    : m_comp(comp)
    , m_layerIndex(layerIndex) {
}

void TimeReverseCommand::execute() {
    const auto& layers = m_comp->getLayers();
    if (m_layerIndex < 0 || m_layerIndex >= static_cast<int>(layers.size())) return;

    auto layer = layers[m_layerIndex];
    m_backups.clear();

    auto processTrack = [&](PropertyTrack& track) {
        if (!track.hasKeyframes()) return;

        PropertyBackup backup;
        backup.track = &track;
        backup.originalKeyframes = track.getKeyframes();
        m_backups.push_back(backup);

        const auto& kfs = track.getKeyframes();
        if (kfs.size() < 2) return;

        double firstTime = kfs.front().getTime();
        double lastTime = kfs.back().getTime();
        double midpoint = (firstTime + lastTime) * 0.5;

        KeyframeList reversed;
        for (auto it = kfs.rbegin(); it != kfs.rend(); ++it) {
            double newTime = firstTime + (lastTime - it->getTime());
            reversed.push_back(Keyframe(newTime, it->getValue(), it->getInterpolation()));
        }

        track = PropertyTrack(track.getName());
        for (const auto& kf : reversed) {
            track.addKeyframe(kf);
        }
    };

    processTrack(layer->getPosition());
    processTrack(layer->getScale());
    processTrack(layer->getRotation());
    processTrack(layer->getOpacity());
    processTrack(layer->getAnchorPoint());

    m_executed = true;
}

void TimeReverseCommand::undo() {
    if (!m_executed) return;

    for (auto& backup : m_backups) {
        if (!backup.track) continue;
        PropertyTrack restored(backup.track->getName());
        for (const auto& kf : backup.originalKeyframes) {
            restored.addKeyframe(kf);
        }
        *backup.track = restored;
    }

    m_backups.clear();
    m_executed = false;
}

std::string TimeReverseCommand::getDescription() const {
    return "Time Reverse Keyframes";
}

} // namespace FreeEffect
