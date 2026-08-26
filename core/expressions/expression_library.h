#pragma once
#include "expression_engine.h"
#include <map>
#include <string>
#include <vector>

namespace FreeEffect {

class ExpressionLibrary {
public:
    static ExpressionLibrary& instance();

    struct ExpressionEntry {
        std::string name;
        std::string category;
        std::string expression;
        std::string description;
    };

    void loadBuiltins();
    const std::vector<ExpressionEntry>& getExpressions() const;
    std::vector<ExpressionEntry> getExpressionsInCategory(const std::string& cat) const;
    std::string getExpression(const std::string& name) const;

    void addCustomExpression(const std::string& name, const std::string& category,
                             const std::string& expression, const std::string& desc = "");

private:
    ExpressionLibrary() { loadBuiltins(); }
    std::vector<ExpressionEntry> m_entries;
};

} // namespace FreeEffect
