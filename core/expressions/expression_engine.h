#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <cmath>
#include <variant>
#include <map>

namespace FreeEffect {

using ExprValue = std::variant<double, std::string, std::vector<double>>;

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
    static ExpressionEngine& instance();

    ExprValue evaluate(const std::string& expression, double time,
                       const std::unordered_map<std::string, ExprValue>& vars = {});
    double evaluate(const std::string& expression, const ExpressionContext& context);
    std::string evaluateString(const std::string& expression, const ExpressionContext& context);

    void setVariable(const std::string& name, double value);
    void setStringVariable(const std::string& name, const std::string& value);
    double getVariable(const std::string& name) const;
    std::string getStringVariable(const std::string& name) const;
    void registerFunction(const std::string& name,
                          std::function<double(const std::vector<double>&)> func);

    // Wiggle
    double wiggle(double freq, double amp, double octaves = 1, double ampMult = 0.5);
    double wiggle3D(double freq, double amp, double octaves = 1);
    double wiggle4D(double freq, double amp, double octaves = 1);
    double wiggleFreq(double time);
    double wiggleAmp(double time);

    // Loop
    double loopOut(const std::string& type = "cycle", double duration = 0);
    double loopIn(const std::string& type = "cycle", double duration = 0);
    double loopOutDuration(const std::string& type = "cycle");
    double loopInDuration(const std::string& type = "cycle");
    double pingpong();
    double loopOffset();

    // Interpolation
    double linear(double value, double inMin, double inMax, double outMin, double outMax);
    double ease(double value, double inMin, double inMax, double outMin, double outMax);
    double easeIn(double value, double inMin, double inMax, double outMin, double outMax);
    double easeOut(double value, double inMin, double inMax, double outMin, double outMax);
    double easeT(double t);
    double smooth(double width, double samples = 5);

    // Math
    double clamp(double value, double minVal, double maxVal);
    double mapRange(double value, double low1, double high1, double low2, double high2);
    double normalize(double value, double minVal, double maxVal);
    double degreesToRadians(double degrees);
    double radiansToDegrees(double radians);
    double vecToAngle(double x, double y);
    double angleToVecX(double angle);
    double angleToVecY(double angle);

    // Vector math
    std::vector<double> addVec(const std::vector<double>& a, const std::vector<double>& b);
    std::vector<double> subVec(const std::vector<double>& a, const std::vector<double>& b);
    std::vector<double> mulVec(const std::vector<double>& a, double s);
    std::vector<double> divVec(const std::vector<double>& a, double s);
    double dotVec(const std::vector<double>& a, const std::vector<double>& b);
    std::vector<double> crossVec(const std::vector<double>& a, const std::vector<double>& b);
    double vecLength(const std::vector<double>& v);
    std::vector<double> normalizeVec(const std::vector<double>& v);
    std::vector<double> interpolateVectors(const std::vector<double>& a,
                                           const std::vector<double>& b, double t);

    // Random
    double random(double max);
    double random2(double min, double max);
    double seedRandom(double seed);
    double gaussRandom(double mean = 0, double stdev = 1);

    // Layer/Comp references
    double layerIndex(const std::string& name);
    double layerInPoint(const std::string& name);
    double layerOutPoint(const std::string& name);
    double layerDuration(const std::string& name);
    std::vector<double> layerPosition(const std::string& name, double t);
    double layerEffectValue(const std::string& layerName, const std::string& effectName,
                            const std::string& paramName, double t);

    // Time
    double getTime() const { return m_currentTime; }
    void setTime(double t) { m_currentTime = t; }

    // sourceRectAtTime
    struct SourceRect { double top = 0, left = 0, width = 0, height = 0; };
    SourceRect sourceRectAtTime(double time = -1);

    // keyframe access
    double key(const std::string& property, int index);
    int numKeys(const std::string& property);

    // Built-in variables
    double value() const { return m_currentValue; }
    double index() const { return m_layerIndex; }

    void setContext(const ExpressionContext& ctx) { m_context = ctx; }
    const ExpressionContext& getContext() const { return m_context; }

private:
    ExpressionEngine();

    ExpressionContext m_context;
    double m_currentTime = 0;
    double m_currentValue = 0;
    int m_layerIndex = 0;

    std::map<std::string, double> m_variables;
    std::map<std::string, std::string> m_stringVariables;
    std::map<std::string, std::function<double(const std::vector<double>&)>> m_functions;
    std::map<std::string, std::vector<std::pair<double, double>>> m_keyframeData;
    std::map<std::string, double> m_layerData;
    std::unordered_map<int, double> m_seedCache;

    void registerBuiltInFunctions();
    double noise1(double x) const;
    double noise2D(double x, double y) const;
    double multiOctaveNoise(double x, int octaves, double ampMult) const;
    double wiggleInternal(double freq, double amp, double octaves, double ampMult,
                          int seedOffset, double t);

    // ---- Lexer ----
    struct Token {
        enum class Type {
            Number, Identifier, String,
            Plus, Minus, Star, Slash, Percent,
            Equals, EqualsEquals, NotEquals,
            Less, LessEqual, Greater, GreaterEqual,
            And, Or, Not,
            Question, Colon, Comma,
            LeftParen, RightParen,
            LeftBracket, RightBracket,
            Dot, Semicolon, Eof
        };
        Type type;
        std::string value;
        double numValue = 0.0;
    };

    class Lexer {
    public:
        explicit Lexer(const std::string& source);
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
    };

    // ---- AST forward declaration ----
    struct ASTNode;

    // ---- AST Evaluator (defined before ASTNode so virtual override resolves) ----
    class ASTEvaluator {
    public:
        ASTEvaluator(ExpressionEngine& engine, const ExpressionContext& ctx)
            : m_engine(engine), m_context(ctx) {}
        double getBuiltInVariable(const std::string& name);
        double evaluateFunction(const std::string& name, const std::vector<double>& args);
        ExpressionEngine& engine() { return m_engine; }
        const ExpressionContext& context() const { return m_context; }
    private:
        ExpressionEngine& m_engine;
        const ExpressionContext& m_context;
    };

    // ---- AST Nodes ----
    struct ASTNode {
        virtual ~ASTNode() = default;
        virtual double eval(ASTEvaluator& e) = 0;
    };
    using ASTNodePtr = std::unique_ptr<ASTNode>;

    struct NumberNode : ASTNode {
        double value;
        explicit NumberNode(double v) : value(v) {}
        double eval(ASTEvaluator&) override { return value; }
    };

    struct StringNode : ASTNode {
        std::string value;
        explicit StringNode(const std::string& v) : value(v) {}
        double eval(ASTEvaluator&) override {
            try { return std::stod(value); } catch (...) { return 0.0; }
        }
    };

    struct VariableNode : ASTNode {
        std::string name;
        explicit VariableNode(const std::string& n) : name(n) {}
        double eval(ASTEvaluator& e) override { return e.getBuiltInVariable(name); }
    };

    struct BinaryOpNode : ASTNode {
        enum Op { Add, Sub, Mul, Div, Mod, Eq, Neq, Lt, Lte, Gt, Gte, And, Or };
        Op op;
        ASTNodePtr left, right;
        BinaryOpNode(Op o, ASTNodePtr l, ASTNodePtr r)
            : op(o), left(std::move(l)), right(std::move(r)) {}
        double eval(ASTEvaluator& e) override {
            double l = left->eval(e);
            double r = right->eval(e);
            switch (op) {
                case Add: return l + r;
                case Sub: return l - r;
                case Mul: return l * r;
                case Div: return (r != 0.0) ? l / r : 0.0;
                case Mod: return (r != 0.0) ? std::fmod(l, r) : 0.0;
                case Eq: return (l == r) ? 1.0 : 0.0;
                case Neq: return (l != r) ? 1.0 : 0.0;
                case Lt: return (l < r) ? 1.0 : 0.0;
                case Lte: return (l <= r) ? 1.0 : 0.0;
                case Gt: return (l > r) ? 1.0 : 0.0;
                case Gte: return (l >= r) ? 1.0 : 0.0;
                case And: return (l != 0.0 && r != 0.0) ? 1.0 : 0.0;
                case Or: return (l != 0.0 || r != 0.0) ? 1.0 : 0.0;
            }
            return 0.0;
        }
    };

    struct UnaryOpNode : ASTNode {
        enum Op { Neg, Not, Positive };
        Op op;
        ASTNodePtr operand;
        UnaryOpNode(Op o, ASTNodePtr v) : op(o), operand(std::move(v)) {}
        double eval(ASTEvaluator& e) override {
            double v = operand->eval(e);
            switch (op) {
                case Neg: return -v;
                case Not: return (v != 0.0) ? 0.0 : 1.0;
                case Positive: return v;
            }
            return 0.0;
        }
    };

    struct TernaryNode : ASTNode {
        ASTNodePtr cond, trueExpr, falseExpr;
        TernaryNode(ASTNodePtr c, ASTNodePtr t, ASTNodePtr f)
            : cond(std::move(c)), trueExpr(std::move(t)), falseExpr(std::move(f)) {}
        double eval(ASTEvaluator& e) override {
            return (cond->eval(e) != 0.0) ? trueExpr->eval(e) : falseExpr->eval(e);
        }
    };

    struct FunctionCallNode : ASTNode {
        std::string fname;
        std::vector<ASTNodePtr> args;
        FunctionCallNode(const std::string& n, std::vector<ASTNodePtr> a)
            : fname(n), args(std::move(a)) {}
        double eval(ASTEvaluator& e) override {
            std::vector<double> evaluated;
            evaluated.reserve(args.size());
            for (auto& a : args) evaluated.push_back(a->eval(e));
            return e.evaluateFunction(fname, evaluated);
        }
    };

    struct MemberAccessNode : ASTNode {
        std::string object;
        std::string property;
        std::string subProperty;
        bool hasSubProperty = false;
        MemberAccessNode(const std::string& obj, const std::string& prop)
            : object(obj), property(prop) {}
        void setSubProperty(const std::string& sp) { subProperty = sp; hasSubProperty = true; }
        double eval(ASTEvaluator& e) override {
            std::string fullName = object + "." + property;
            if (hasSubProperty) fullName += "." + subProperty;
            return e.getBuiltInVariable(fullName);
        }
    };

    struct AssignNode : ASTNode {
        std::string name;
        ASTNodePtr value;
        AssignNode(const std::string& n, ASTNodePtr v) : name(n), value(std::move(v)) {}
        double eval(ASTEvaluator& e) override {
            double v = value->eval(e);
            e.engine().m_variables[name] = v;
            return v;
        }
    };

    struct CommaExprNode : ASTNode {
        std::vector<ASTNodePtr> expressions;
        explicit CommaExprNode(std::vector<ASTNodePtr> exprs) : expressions(std::move(exprs)) {}
        double eval(ASTEvaluator& e) override {
            double result = 0.0;
            for (auto& ex : expressions) result = ex->eval(e);
            return result;
        }
    };

    // ---- Parser ----
    class Parser {
    public:
        Parser(Lexer& lexer, ExpressionEngine& engine);
        ASTNodePtr parseExpression();
        std::string parseStringExpression();
    private:
        ASTNodePtr parseTernary();
        ASTNodePtr parseOr();
        ASTNodePtr parseAnd();
        ASTNodePtr parseComparison();
        ASTNodePtr parseAddSub();
        ASTNodePtr parseMulDiv();
        ASTNodePtr parseUnary();
        ASTNodePtr parsePrimary();
        std::string parseStringPrimary();
        std::string parseStringConcat();
        Lexer& m_lexer;
        ExpressionEngine& m_engine;
        Token m_current;
    };
};

} // namespace FreeEffect
