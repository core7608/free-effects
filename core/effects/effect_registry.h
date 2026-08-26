#pragma once

#include "effect.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace FreeEffect {

using EffectFactory = std::function<std::unique_ptr<Effect>()>;

class EffectRegistry {
public:
    static EffectRegistry& instance();

    void registerEffect(const std::string& name, EffectFactory factory,
                        const std::string& category, const std::string& subCategory = "");

    std::unique_ptr<Effect> create(const std::string& name) const;
    bool hasEffect(const std::string& name) const;

    std::vector<std::string> getEffectNames() const;
    std::vector<std::string> getCategories() const;
    std::vector<std::string> getEffectNamesInCategory(const std::string& category) const;
    std::vector<std::string> getEffectNamesInSubCategory(const std::string& category,
                                                         const std::string& subCategory) const;

    struct EffectInfo {
        std::string name;
        std::string category;
        std::string subCategory;
        EffectFactory factory;
    };

    const std::vector<EffectInfo>& getAllEffects() const { return m_effects; }

private:
    EffectRegistry() = default;
    std::vector<EffectInfo> m_effects;
    std::unordered_map<std::string, int> m_nameIndex;
};

template<typename T>
struct EffectRegistrar {
    EffectRegistrar(const std::string& name, const std::string& category,
                    const std::string& subCategory = "") {
        EffectRegistry::instance().registerEffect(
            name, []() -> std::unique_ptr<Effect> { return std::make_unique<T>(); },
            category, subCategory);
    }
};

} // namespace FreeEffect
