#pragma once

#include "command.h"
#include "../timeline/composition.h"
#include "../timeline/layer.h"
#include "../timeline/shape_layer.h"
#include <vector>

namespace FreeEffect {

enum class TraceMode { Alpha, Luminance, Channels };

class AutoTraceCommand : public Command {
public:
    AutoTraceCommand(Composition* comp, int layerIndex, double time,
                     TraceMode mode = TraceMode::Alpha,
                     int threshold = 50,
                     double tolerance = 5.0,
                     bool invert = false);

    void execute() override;
    void undo() override;
    std::string getDescription() const override;

    const std::vector<ShapePath>& getTracedPaths() const { return m_tracedPaths; }

private:
    Composition* m_comp;
    int m_layerIndex;
    double m_time;
    TraceMode m_mode;
    int m_threshold;
    double m_tolerance;
    bool m_invert;

    std::vector<ShapePath> m_tracedPaths;
    LayerPtr m_maskLayer;
    bool m_executed = false;

    void traceFromBuffer(int width, int height,
                         const uint8_t* data, int channels);
    void marchingSquares(const std::vector<bool>& grid,
                         int gridW, int gridH,
                         double cellW, double cellH,
                         double offsetX, double offsetY);
};

} // namespace FreeEffect
