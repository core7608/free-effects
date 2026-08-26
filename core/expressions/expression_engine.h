#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <variant>
#include <cmath>

namespace FreeEffect {

struct ExpressionContext {
    double time = 0.0;
    double value = 0.0;
    int index = 1;
    int numLayers = 1;
    double compWidth = 1920.0;
    double compHeight = 1080.0;
    double fps = 30.0;
    double startTime = 0.0;
    double endTime = 10.0;
    int loopCount = 0;
};

class ExpressionEngine {
public:
    ExpressionEngine();
    ~ExpressionEngine() = default;

    double evaluate(const std::string& expression, const ExpressionContext& context);
    std::string evaluateString(const std::string& expression, const ExpressionContext& context);

    void setVariable(const std::string& name, double value);
    void setStringVariable(const std::string& name, const std::string& value);
    double getVariable(const std::string& name) const;
    std::string getStringVariable(const std::string& name) const;

    void registerFunction(const std::string& name, std::function<double(const std::vector<double>&)> func);

private:
    struct Token {
        enum class Type {
            Number,
            Identifier,
            Plus,
            Minus,
            Star,
            Slash,
            Percent,
            Equals,
            EqualsEquals,
            NotEquals,
            Less,
            LessEqual,
            Greater,
            GreaterEqual,
            And,
            Or,
            Not,
            Question,
            Colon,
            Comma,
            LeftParen,
            RightParen,
            LeftBracket,
            RightBracket,
            Dot,
            String,
            Eof
        };

        Type type;
        std::string value;
        double numValue = 0.0;
    };

    class Lexer {
    public:
        Lexer(const std::string& source, const ExpressionContext& ctx);
        Token next();
        Token peek();
        void advance();

    private:
        void skipWhitespace();
        Token readNumber();
        Token readIdentifier();
        Token readString();

        std::string m_source;
        size_t m_pos = 0;
        std::vector<Token> m_peeked;
        const ExpressionContext& m_context;
    };

    class Parser {
    public:
        Parser(Lexer& lexer, ExpressionEngine& engine, const ExpressionContext& ctx);

        double parseExpression();
        std::string parseStringExpression();

    private:
        double parseTernary();
        double parseOr();
        double parseAnd();
        double parseComparison();
        double parseAddSub();
        double parseMulDiv();
        double parseUnary();
        double parsePrimary();

        std::string parseStringPrimary();
        std::string parseStringConcat();

        double evaluateFunction(const std::string& name, std::vector<double> args);
        double getBuiltInVariable(const std::string& name);

        Lexer& m_lexer;
        ExpressionEngine& m_engine;
        const ExpressionContext& m_context;
        Token m_current;
    };

    std::map<std::string, double> m_variables;
    std::map<std::string, std::string> m_stringVariables;
    std::map<std::string, std::function<double(const std::vector<double>&)>> m_functions;

    void registerBuiltInFunctions();
};

} // namespace FreeEffect
