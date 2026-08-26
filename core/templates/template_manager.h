#pragma once

#include "template_data.h"
#include "../timeline/composition.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace FreeEffect {

class TemplateManager {
public:
    TemplateManager();
    ~TemplateManager() = default;

    void loadFromDirectory(const std::string& dirPath);
    void loadFromFile(const std::string& filePath);

    const TemplateData* getTemplate(const std::string& name) const;
    std::vector<std::string> getCategories() const;
    std::vector<std::string> getTemplateNames(const std::string& category = "") const;
    std::vector<const TemplateData*> getTemplatesByCategory(const std::string& category) const;

    std::unique_ptr<Composition> createComposition(const std::string& templateName, const std::string& compName = "");

private:
    void parseTemplateJson(const std::string& jsonStr);
    void parseTemplateLayer(const void* layerObj, TemplateLayer& layer);
    void parseTemplateEffect(const void* effectObj, TemplateEffect& effect);

    std::unordered_map<std::string, TemplateData> m_templates;
};

} // namespace FreeEffect
