#include "effect_registry.h"

namespace FreeEffect {

EffectRegistry& EffectRegistry::instance() {
    static EffectRegistry reg;
    return reg;
}

void EffectRegistry::registerEffect(const std::string& name, EffectFactory factory,
                                    const std::string& category, const std::string& subCategory) {
    auto it = m_nameIndex.find(name);
    if (it != m_nameIndex.end()) {
        m_effects[it->second].factory = std::move(factory);
        m_effects[it->second].category = category;
        m_effects[it->second].subCategory = subCategory;
        return;
    }
    m_nameIndex[name] = static_cast<int>(m_effects.size());
    m_effects.push_back({name, category, subCategory, std::move(factory)});
}

std::unique_ptr<Effect> EffectRegistry::create(const std::string& name) const {
    auto it = m_nameIndex.find(name);
    if (it != m_nameIndex.end()) {
        return m_effects[it->second].factory();
    }
    return nullptr;
}

bool EffectRegistry::hasEffect(const std::string& name) const {
    return m_nameIndex.find(name) != m_nameIndex.end();
}

std::vector<std::string> EffectRegistry::getEffectNames() const {
    std::vector<std::string> names;
    names.reserve(m_effects.size());
    for (const auto& e : m_effects) {
        names.push_back(e.name);
    }
    return names;
}

std::vector<std::string> EffectRegistry::getCategories() const {
    std::vector<std::string> cats;
    for (const auto& e : m_effects) {
        bool found = false;
        for (const auto& c : cats) {
            if (c == e.category) { found = true; break; }
        }
        if (!found) cats.push_back(e.category);
    }
    return cats;
}

std::vector<std::string> EffectRegistry::getEffectNamesInCategory(const std::string& category) const {
    std::vector<std::string> names;
    for (const auto& e : m_effects) {
        if (e.category == category) names.push_back(e.name);
    }
    return names;
}

std::vector<std::string> EffectRegistry::getEffectNamesInSubCategory(
    const std::string& category, const std::string& subCategory) const {
    std::vector<std::string> names;
    for (const auto& e : m_effects) {
        if (e.category == category && e.subCategory == subCategory) names.push_back(e.name);
    }
    return names;
}

} // namespace FreeEffect
