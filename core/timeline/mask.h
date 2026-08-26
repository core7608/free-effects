#pragma once

#include "../math/matrix4.h"
#include "../rendering/renderer.h"
#include "keyframe_interpolation.h"
#include "property_track.h"
#include "types.h"
#include <vector>

namespace FreeEffect {

struct MaskVertex {
    Vec2 position;
    BezierHandle inHandle;
    BezierHandle outHandle;
};

enum class MaskMode {
    None,
    Add,
    Subtract,
    Intersect,
    Lighten,
    Darken,
    Difference
};

class Mask {
public:
    Mask();
    Mask(const std::string& name, MaskMode mode = MaskMode::Add);

    const UUID& getId() const { return m_id; }

    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    MaskMode getMode() const { return m_mode; }
    void setMode(MaskMode mode) { m_mode = mode; }

    bool isInverted() const { return m_inverted; }
    void setInverted(bool inverted) { m_inverted = inverted; }

    PropertyTrack& getOpacity() { return m_opacity; }
    const PropertyTrack& getOpacity() const { return m_opacity; }

    std::vector<MaskVertex>& getVertices() { return m_vertices; }
    const std::vector<MaskVertex>& getVertices() const { return m_vertices; }

    void addVertex(const MaskVertex& vertex);
    void removeVertex(int index);
    void setVertexPosition(int index, const Vec2& position);

    bool isPointInside(double x, double y) const;
    PixelBuffer applyMask(const PixelBuffer& input) const;

private:
    bool pointOnCurveSegment(double px, double py,
                             const Vec2& p0, const BezierHandle& h0,
                             const Vec2& p1, const BezierHandle& h1) const;
    bool isPointOnBoundary(double x, double y, double tolerance = 0.5) const;

    UUID m_id;
    std::string m_name;
    MaskMode m_mode = MaskMode::Add;
    bool m_inverted = false;
    PropertyTrack m_opacity;
    std::vector<MaskVertex> m_vertices;
};

} // namespace FreeEffect
