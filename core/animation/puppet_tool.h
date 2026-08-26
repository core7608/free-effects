#pragma once
#include "../rendering/renderer.h"
#include "../timeline/types.h"
#include <vector>
#include <array>

namespace FreeEffect {

struct PuppetPin {
    double x = 0, y = 0;
    double offsetX = 0, offsetY = 0;
    int meshIndex = 0;
    bool isBase = false;
};

struct PuppetMesh {
    int resolution = 5;
    std::vector<Vec2> vertices;
    std::vector<std::array<int, 3>> triangles;
    double expansion = 1.0;

    void generateMesh(int width, int height);
    void deform(const std::vector<PuppetPin>& pins, int width, int height);
};

class PuppetTool {
public:
    void addPin(double x, double y, bool isBase = false);
    void removePin(int index);
    void setPinPosition(int index, double x, double y);
    void setPinOffset(int index, double offsetX, double offsetY);

    void setMeshResolution(int resolution) { m_meshResolution = resolution; }
    void setExpansion(double pixels) { m_expansion = pixels; }

    const std::vector<PuppetPin>& getPins() const { return m_pins; }

    PixelBuffer deformBuffer(const PixelBuffer& source, double time) const;

private:
    std::vector<PuppetPin> m_pins;
    int m_meshResolution = 5;
    double m_expansion = 1.0;

    PuppetMesh generateMesh(int width, int height) const;
    void applyDeformation(PixelBuffer& target, const PixelBuffer& source,
                          const PuppetMesh& mesh, int width, int height) const;
};

} // namespace FreeEffect
