#include "solids_library.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace FreeEffect {

bool SolidsLibrary::colorsMatch(const Color& a, const Color& b) {
    const double epsilon = 0.001;
    return std::abs(a.r - b.r) < epsilon &&
           std::abs(a.g - b.g) < epsilon &&
           std::abs(a.b - b.b) < epsilon &&
           std::abs(a.a - b.a) < epsilon;
}

void SolidsLibrary::addSolid(const SolidDef& solid) {
    auto it = std::find_if(m_solids.begin(), m_solids.end(),
        [&solid](const SolidDef& s) { return s.name == solid.name; });
    if (it == m_solids.end()) {
        m_solids.push_back(solid);
    } else {
        *it = solid;
    }
}

void SolidsLibrary::removeSolid(const std::string& name) {
    m_solids.erase(
        std::remove_if(m_solids.begin(), m_solids.end(),
            [&name](const SolidDef& s) { return s.name == name; }),
        m_solids.end());
}

SolidDef SolidsLibrary::getSolid(const std::string& name) const {
    auto it = std::find_if(m_solids.begin(), m_solids.end(),
        [&name](const SolidDef& s) { return s.name == name; });
    if (it != m_solids.end()) return *it;
    return {};
}

std::string SolidsLibrary::findOrCreateSolid(const Color& color, int width, int height) {
    for (const auto& solid : m_solids) {
        if (colorsMatch(solid.color, color) &&
            solid.width == width && solid.height == height) {
            return solid.name;
        }
    }

    std::ostringstream oss;
    oss << "Solid " << m_solids.size() + 1;
    oss << " [";
    oss << static_cast<int>(color.r * 255) << ","
        << static_cast<int>(color.g * 255) << ","
        << static_cast<int>(color.b * 255);
    oss << "] " << width << "x" << height;

    SolidDef newSolid;
    newSolid.name = oss.str();
    newSolid.color = color;
    newSolid.width = width;
    newSolid.height = height;
    newSolid.usedCount = 1;
    m_solids.push_back(newSolid);
    return newSolid.name;
}

void SolidsLibrary::replaceAllInstances(const std::string& oldName, const SolidDef& newSolid) {
    auto it = std::find_if(m_solids.begin(), m_solids.end(),
        [&oldName](const SolidDef& s) { return s.name == oldName; });
    if (it != m_solids.end()) {
        int usage = it->usedCount;
        *it = newSolid;
        it->usedCount = usage;
    }
}

std::vector<SolidDef> SolidsLibrary::getUnusedSolids() const {
    std::vector<SolidDef> unused;
    std::copy_if(m_solids.begin(), m_solids.end(), std::back_inserter(unused),
        [](const SolidDef& s) { return s.usedCount <= 0; });
    return unused;
}

void SolidsLibrary::removeUnusedSolids() {
    m_solids.erase(
        std::remove_if(m_solids.begin(), m_solids.end(),
            [](const SolidDef& s) { return s.usedCount <= 0; }),
        m_solids.end());
}

void SolidsLibrary::incrementUsage(const std::string& name) {
    auto it = std::find_if(m_solids.begin(), m_solids.end(),
        [&name](const SolidDef& s) { return s.name == name; });
    if (it != m_solids.end()) {
        it->usedCount++;
    }
}

int SolidsLibrary::getUsageCount(const std::string& name) const {
    auto it = std::find_if(m_solids.begin(), m_solids.end(),
        [&name](const SolidDef& s) { return s.name == name; });
    if (it != m_solids.end()) return it->usedCount;
    return 0;
}

} // namespace FreeEffect
