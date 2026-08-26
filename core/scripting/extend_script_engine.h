#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <variant>
#include <memory>
#include <mutex>

namespace FreeEffect {

using ScriptValue = std::variant<double, std::string, std::vector<double>, bool>;

class ExtendScriptEngine {
public:
    static ExtendScriptEngine& instance();

    bool execute(const std::string& script);
    bool executeFile(const std::string& filePath);

    void setVariable(const std::string& name, const ScriptValue& value);
    ScriptValue getVariable(const std::string& name) const;

    void registerFunction(const std::string& name,
        std::function<ScriptValue(const std::vector<ScriptValue>&)> func);

    struct ProjectAPI {
        int numItems() const;
        std::string item(int index) const;
        std::string getActiveItem() const;
        int activeCompIndex() const;
        void openProject(const std::string& path);
        void saveProject(const std::string& path);
        void closeProject();
    };

    struct CompAPI {
        int numLayers() const;
        std::string layer(int index) const;
        double duration() const;
        double frameRate() const;
        int width() const;
        int height() const;
        void addLayer(const std::string& type, double duration);
    };

    struct LayerAPI {
        std::string name() const;
        void setName(const std::string& n);
        double inPoint() const;
        double outPoint() const;
        void setInPoint(double t);
        void setOutPoint(double t);
        double opacity() const;
        void setOpacity(double v);
        std::vector<double> position() const;
        void setPosition(const std::vector<double>& pos);
        std::vector<double> scale() const;
        void setScale(const std::vector<double>& s);
        double rotation() const;
        void setRotation(double r);
        int numEffects() const;
        void addEffect(const std::string& name);
    };

    struct RenderQueueAPI {
        int numItems() const;
        void addItem(const std::string& compName);
        void setOutputModule(int item, const std::string& module);
        void setOutputPath(int item, const std::string& path);
        void startRendering();
        void stopRendering();
    };

    ProjectAPI project;
    CompAPI comp;
    LayerAPI layer;
    RenderQueueAPI renderQueue;

    void setLogCallback(std::function<void(const std::string&)> callback);
    void log(const std::string& message);
    void clearOutput();

    bool hasError() const;
    std::string getLastError() const;

private:
    ExtendScriptEngine() = default;

    std::unordered_map<std::string, ScriptValue> m_variables;
    std::unordered_map<std::string, std::function<ScriptValue(const std::vector<ScriptValue>&)>> m_functions;
    std::function<void(const std::string&)> m_logCallback;
    std::string m_lastError;
    mutable std::mutex m_mutex2;

    ScriptValue evalExpression(const std::string& expr);

    // Tokenizer
    enum class TokenType {
        Number, String, Identifier,
        Plus, Minus, Star, Slash, Percent,
        Equals, EqualsEquals, NotEquals,
        Less, LessEqual, Greater, GreaterEqual,
        And, Or, Not,
        LeftParen, RightParen,
        LeftBracket, RightBracket,
        Comma, Dot, Semicolon, Eof
    };

    struct Token {
        TokenType type;
        std::string value;
        double numValue = 0.0;
    };

    class Tokenizer {
    public:
        explicit Tokenizer(const std::string& src);
        Token next();
        Token peek();
    private:
        void skipWhitespace();
        Token readNumber();
        Token readString();
        Token readIdentifier();
        std::string m_src;
        size_t m_pos = 0;
        std::vector<Token> m_peeked;
    };

    ScriptValue parseExpression(Tokenizer& tok);
    ScriptValue parseOr(Tokenizer& tok);
    ScriptValue parseAnd(Tokenizer& tok);
    ScriptValue parseComparison(Tokenizer& tok);
    ScriptValue parseAddSub(Tokenizer& tok);
    ScriptValue parseMulDiv(Tokenizer& tok);
    ScriptValue parseUnary(Tokenizer& tok);
    ScriptValue parsePrimary(Tokenizer& tok);
    ScriptValue parseFunctionCall(const std::string& name, Tokenizer& tok);
    ScriptValue parseArrayLiteral(Tokenizer& tok);
};

} // namespace FreeEffect
