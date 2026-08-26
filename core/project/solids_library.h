#pragma once
#include <string>
#include <vector>
#include "../timeline/types.h"

namespace FreeEffect {

struct SolidDef {
    std::string name;
    Color color;
    int width = 1920;
    int height = 1080;
    std::string pixelAspect = "Square Pixels";
    int usedCount = 0;
};

class SolidsLibrary {
public:
    void addSolid(const SolidDef& solid);
    void removeSolid(const std::string& name);
    SolidDef getSolid(const std::string& name) const;
    std::vector<SolidDef> getAllSolids() const { return m_solids; }

    std::string findOrCreateSolid(const Color& color, int width, int height);
    void replaceAllInstances(const std::string& oldName, const SolidDef& newSolid);

    std::vector<SolidDef> getUnusedSolids() const;
    void removeUnusedSolids();

    void incrementUsage(const std::string& name);
    int getUsageCount(const std::string& name) const;

private:
    std::vector<SolidDef> m_solids;

    static bool colorsMatch(const Color& a, const Color& b);
};

} // namespace FreeEffect
