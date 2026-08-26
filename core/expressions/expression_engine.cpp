#include "expression_engine.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <random>

namespace FreeEffect {

ExpressionEngine::ExpressionEngine() {
    registerBuiltInFunctions();
}

void ExpressionEngine::setVariable(const std::string& name, double value) {
    m_variables[name] = value;
}

void ExpressionEngine::setStringVariable(const std::string& name, const std::string& value) {
    m_stringVariables[name] = value;
}

double ExpressionEngine::getVariable(const std::string& name) const {
    auto it = m_variables.find(name);
    return (it != m_variables.end()) ? it->second : 0.0;
}

std::string ExpressionEngine::getStringVariable(const std::string& name) const {
    auto it = m_stringVariables.find(name);
    return (it != m_stringVariables.end()) ? it->second : "";
}

void ExpressionEngine::registerFunction(const std::string& name, std::function<double(const std::vector<double>&)> func) {
    m_functions[name] = std::move(func);
}

double ExpressionEngine::evaluate(const std::string& expression, const ExpressionContext& context) {
    Lexer lexer(expression, context);
    Parser parser(lexer, *this, context);
    return parser.parseExpression();
}

std::string ExpressionEngine::evaluateString(const std::string& expression, const ExpressionContext& context) {
    Lexer lexer(expression, context);
    Parser parser(lexer, *this, context);
    return parser.parseStringExpression();
}

void ExpressionEngine::registerBuiltInFunctions() {
    m_functions["sin"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::sin(args[0]) : 0.0;
    };
    m_functions["cos"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::cos(args[0]) : 0.0;
    };
    m_functions["tan"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::tan(args[0]) : 0.0;
    };
    m_functions["asin"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::asin(args[0]) : 0.0;
    };
    m_functions["acos"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::acos(args[0]) : 0.0;
    };
    m_functions["atan"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::atan(args[0]) : 0.0;
    };
    m_functions["atan2"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 2) ? std::atan2(args[0], args[1]) : 0.0;
    };
    m_functions["abs"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::abs(args[0]) : 0.0;
    };
    m_functions["min"] = [](const std::vector<double>& args) -> double {
        if (args.empty()) return 0.0;
        double m = args[0];
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i] < m) m = args[i];
        }
        return m;
    };
    m_functions["max"] = [](const std::vector<double>& args) -> double {
        if (args.empty()) return 0.0;
        double m = args[0];
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i] > m) m = args[i];
        }
        return m;
    };
    m_functions["clamp"] = [](const std::vector<double>& args) -> double {
        if (args.size() >= 3) {
            if (args[0] < args[1]) return args[1];
            if (args[0] > args[2]) return args[2];
            return args[0];
        }
        return (args.size() >= 1) ? args[0] : 0.0;
    };
    m_functions["floor"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::floor(args[0]) : 0.0;
    };
    m_functions["ceil"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::ceil(args[0]) : 0.0;
    };
    m_functions["round"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::round(args[0]) : 0.0;
    };
    m_functions["frac"] = [](const std::vector<double>& args) -> double {
        if (args.size() < 1) return 0.0;
        return args[0] - std::floor(args[0]);
    };
    m_functions["sign"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? ((args[0] > 0.0) ? 1.0 : ((args[0] < 0.0) ? -1.0 : 0.0)) : 0.0;
    };
    m_functions["pow"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 2) ? std::pow(args[0], args[1]) : 0.0;
    };
    m_functions["sqrt"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::sqrt(args[0]) : 0.0;
    };
    m_functions["exp"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::exp(args[0]) : 0.0;
    };
    m_functions["log"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::log(args[0]) : 0.0;
    };
    m_functions["log2"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::log2(args[0]) : 0.0;
    };
    m_functions["degToRad"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? args[0] * 3.14159265358979323846 / 180.0 : 0.0;
    };
    m_functions["radToDeg"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? args[0] * 180.0 / 3.14159265358979323846 : 0.0;
    };
    m_functions["random"] = [](const std::vector<double>& args) -> double {
        static std::mt19937 rng((std::random_device())());
        if (args.size() >= 2) {
            std::uniform_real_distribution<double> dist(args[0], args[1]);
            return dist(rng);
        }
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(rng);
    };
    m_functions["seedRandom"] = [](const std::vector<double>& args) -> double {
        return (args.size() >= 1) ? std::fmod(std::sin(args[0]) * 43758.5453, 1.0) : 0.0;
    };
    m_functions["length"] = [](const std::vector<double>& args) -> double {
        if (args.size() >= 2) {
            return std::sqrt(args[0] * args[0] + args[1] * args[1]);
        }
        if (args.size() >= 3) {
            return std::sqrt(args[0] * args[0] + args[1] * args[1] + args[2] * args[2]);
        }
        return 0.0;
    };
    m_functions["dot"] = [](const std::vector<double>& args) -> double {
        if (args.size() >= 4) {
            return args[0] * args[2] + args[1] * args[3];
        }
        if (args.size() >= 6) {
            return args[0] * args[3] + args[1] * args[4] + args[2] * args[5];
        }
        return 0.0;
    };
    m_functions["normalize"] = [](const std::vector<double>& args) -> double {
        if (args.size() >= 2) {
            double len = std::sqrt(args[0] * args[0] + args[1] * args[1]);
            return (len > 0.0) ? args[0] / len : 0.0;
        }
        return 0.0;
    };
}

ExpressionEngine::Lexer::Lexer(const std::string& source, const ExpressionContext& ctx)
    : m_source(source), m_context(ctx) {
    m_pos = 0;
}

void ExpressionEngine::Lexer::skipWhitespace() {
    while (m_pos < m_source.size() && (m_source[m_pos] == ' ' || m_source[m_pos] == '\t' || m_source[m_pos] == '\r' || m_source[m_pos] == '\n')) {
        m_pos++;
    }
}

ExpressionEngine::Token ExpressionEngine::Lexer::readNumber() {
    Token tok;
    tok.type = Token::Type::Number;
    size_t start = m_pos;
    bool hasDot = false;
    while (m_pos < m_source.size() && (std::isdigit(m_source[m_pos]) || m_source[m_pos] == '.')) {
        if (m_source[m_pos] == '.') {
            if (hasDot) break;
            hasDot = true;
        }
        m_pos++;
    }
    tok.value = m_source.substr(start, m_pos - start);
    tok.numValue = std::stod(tok.value);
    return tok;
}

ExpressionEngine::Token ExpressionEngine::Lexer::readIdentifier() {
    Token tok;
    tok.type = Token::Type::Identifier;
    size_t start = m_pos;
    while (m_pos < m_source.size() && (std::isalnum(m_source[m_pos]) || m_source[m_pos] == '_')) {
        m_pos++;
    }
    tok.value = m_source.substr(start, m_pos - start);
    return tok;
}

ExpressionEngine::Token ExpressionEngine::Lexer::readString() {
    Token tok;
    tok.type = Token::Type::String;
    m_pos++;
    size_t start = m_pos;
    while (m_pos < m_source.size() && m_source[m_pos] != '"') {
        if (m_source[m_pos] == '\\') m_pos++;
        m_pos++;
    }
    tok.value = m_source.substr(start, m_pos - start);
    if (m_pos < m_source.size()) m_pos++;
    return tok;
}

void ExpressionEngine::Lexer::advance() {
    if (!m_peeked.empty()) {
        m_peeked.erase(m_peeked.begin());
    }
}

ExpressionEngine::Token ExpressionEngine::Lexer::peek() {
    if (!m_peeked.empty()) {
        return m_peeked.back();
    }
    Token tok = next();
    m_peeked.push_back(tok);
    return tok;
}

ExpressionEngine::Token ExpressionEngine::Lexer::next() {
    if (!m_peeked.empty()) {
        Token tok = m_peeked.back();
        m_peeked.pop_back();
        return tok;
    }

    skipWhitespace();
    if (m_pos >= m_source.size()) {
        Token tok;
        tok.type = Token::Type::Eof;
        return tok;
    }

    char c = m_source[m_pos];

    if (std::isdigit(c) || (c == '.' && m_pos + 1 < m_source.size() && std::isdigit(m_source[m_pos + 1]))) {
        return readNumber();
    }

    if (std::isalpha(c) || c == '_') {
        return readIdentifier();
    }

    if (c == '"') {
        return readString();
    }

    Token tok;
    m_pos++;
    switch (c) {
        case '+': tok.type = Token::Type::Plus; tok.value = "+"; break;
        case '-': tok.type = Token::Type::Minus; tok.value = "-"; break;
        case '*': tok.type = Token::Type::Star; tok.value = "*"; break;
        case '/': tok.type = Token::Type::Slash; tok.value = "/"; break;
        case '%': tok.type = Token::Type::Percent; tok.value = "%"; break;
        case '(': tok.type = Token::Type::LeftParen; tok.value = "("; break;
        case ')': tok.type = Token::Type::RightParen; tok.value = ")"; break;
        case '[': tok.type = Token::Type::LeftBracket; tok.value = "["; break;
        case ']': tok.type = Token::Type::RightBracket; tok.value = "]"; break;
        case '.': tok.type = Token::Type::Dot; tok.value = "."; break;
        case ',': tok.type = Token::Type::Comma; tok.value = ","; break;
        case ':': tok.type = Token::Type::Colon; tok.value = ":"; break;
        case '?': tok.type = Token::Type::Question; tok.value = "?"; break;
        case '=':
            if (m_pos < m_source.size() && m_source[m_pos] == '=') {
                m_pos++;
                tok.type = Token::Type::EqualsEquals;
                tok.value = "==";
            } else {
                tok.type = Token::Type::Equals;
                tok.value = "=";
            }
            break;
        case '!':
            if (m_pos < m_source.size() && m_source[m_pos] == '=') {
                m_pos++;
                tok.type = Token::Type::NotEquals;
                tok.value = "!=";
            } else {
                tok.type = Token::Type::Not;
                tok.value = "!";
            }
            break;
        case '<':
            if (m_pos < m_source.size() && m_source[m_pos] == '=') {
                m_pos++;
                tok.type = Token::Type::LessEqual;
                tok.value = "<=";
            } else {
                tok.type = Token::Type::Less;
                tok.value = "<";
            }
            break;
        case '>':
            if (m_pos < m_source.size() && m_source[m_pos] == '=') {
                m_pos++;
                tok.type = Token::Type::GreaterEqual;
                tok.value = ">=";
            } else {
                tok.type = Token::Type::Greater;
                tok.value = ">";
            }
            break;
        case '&':
            if (m_pos < m_source.size() && m_source[m_pos] == '&') {
                m_pos++;
                tok.type = Token::Type::And;
                tok.value = "&&";
            }
            break;
        case '|':
            if (m_pos < m_source.size() && m_source[m_pos] == '|') {
                m_pos++;
                tok.type = Token::Type::Or;
                tok.value = "||";
            }
            break;
        default:
            tok.type = Token::Type::Eof;
            break;
    }
    return tok;
}

ExpressionEngine::Parser::Parser(Lexer& lexer, ExpressionEngine& engine, const ExpressionContext& ctx)
    : m_lexer(lexer), m_engine(engine), m_context(ctx) {
    m_current = m_lexer.next();
}

double ExpressionEngine::Parser::parseExpression() {
    return parseTernary();
}

std::string ExpressionEngine::Parser::parseStringExpression() {
    return parseStringConcat();
}

double ExpressionEngine::Parser::parseTernary() {
    double left = parseOr();
    if (m_current.type == Token::Type::Question) {
        m_current = m_lexer.next();
        double trueVal = parseExpression();
        if (m_current.type == Token::Type::Colon) {
            m_current = m_lexer.next();
        }
        double falseVal = parseExpression();
        return left ? trueVal : falseVal;
    }
    return left;
}

double ExpressionEngine::Parser::parseOr() {
    double left = parseAnd();
    while (m_current.type == Token::Type::Or) {
        m_current = m_lexer.next();
        double right = parseAnd();
        left = (left || right) ? 1.0 : 0.0;
    }
    return left;
}

double ExpressionEngine::Parser::parseAnd() {
    double left = parseComparison();
    while (m_current.type == Token::Type::And) {
        m_current = m_lexer.next();
        double right = parseComparison();
        left = (left && right) ? 1.0 : 0.0;
    }
    return left;
}

double ExpressionEngine::Parser::parseComparison() {
    double left = parseAddSub();
    while (m_current.type == Token::Type::EqualsEquals || m_current.type == Token::Type::NotEquals ||
           m_current.type == Token::Type::Less || m_current.type == Token::Type::LessEqual ||
           m_current.type == Token::Type::Greater || m_current.type == Token::Type::GreaterEqual) {
        Token::Type op = m_current.type;
        m_current = m_lexer.next();
        double right = parseAddSub();
        switch (op) {
            case Token::Type::EqualsEquals: left = (left == right) ? 1.0 : 0.0; break;
            case Token::Type::NotEquals: left = (left != right) ? 1.0 : 0.0; break;
            case Token::Type::Less: left = (left < right) ? 1.0 : 0.0; break;
            case Token::Type::LessEqual: left = (left <= right) ? 1.0 : 0.0; break;
            case Token::Type::Greater: left = (left > right) ? 1.0 : 0.0; break;
            case Token::Type::GreaterEqual: left = (left >= right) ? 1.0 : 0.0; break;
            default: break;
        }
    }
    return left;
}

double ExpressionEngine::Parser::parseAddSub() {
    double left = parseMulDiv();
    while (m_current.type == Token::Type::Plus || m_current.type == Token::Type::Minus) {
        bool isAdd = (m_current.type == Token::Type::Plus);
        m_current = m_lexer.next();
        double right = parseMulDiv();
        left = isAdd ? (left + right) : (left - right);
    }
    return left;
}

double ExpressionEngine::Parser::parseMulDiv() {
    double left = parseUnary();
    while (m_current.type == Token::Type::Star || m_current.type == Token::Type::Slash || m_current.type == Token::Type::Percent) {
        Token::Type op = m_current.type;
        m_current = m_lexer.next();
        double right = parseUnary();
        switch (op) {
            case Token::Type::Star: left = left * right; break;
            case Token::Type::Slash: left = (right != 0.0) ? (left / right) : 0.0; break;
            case Token::Type::Percent: left = (right != 0.0) ? std::fmod(left, right) : 0.0; break;
            default: break;
        }
    }
    return left;
}

double ExpressionEngine::Parser::parseUnary() {
    if (m_current.type == Token::Type::Minus) {
        m_current = m_lexer.next();
        return -parseUnary();
    }
    if (m_current.type == Token::Type::Plus) {
        m_current = m_lexer.next();
        return parseUnary();
    }
    if (m_current.type == Token::Type::Not) {
        m_current = m_lexer.next();
        return parseUnary() ? 0.0 : 1.0;
    }
    return parsePrimary();
}

double ExpressionEngine::Parser::parsePrimary() {
    if (m_current.type == Token::Type::Number) {
        double val = m_current.numValue;
        m_current = m_lexer.next();
        return val;
    }

    if (m_current.type == Token::Type::LeftParen) {
        m_current = m_lexer.next();
        double val = parseExpression();
        if (m_current.type == Token::Type::RightParen) {
            m_current = m_lexer.next();
        }
        return val;
    }

    if (m_current.type == Token::Type::LeftBracket) {
        m_current = m_lexer.next();
        double idx = parseExpression();
        if (m_current.type == Token::Type::RightBracket) {
            m_current = m_lexer.next();
        }
        return idx;
    }

    if (m_current.type == Token::Type::Identifier) {
        std::string name = m_current.value;
        m_current = m_lexer.next();

        if (m_current.type == Token::Type::LeftParen) {
            m_current = m_lexer.next();
            std::vector<double> args;
            if (m_current.type != Token::Type::RightParen) {
                args.push_back(parseExpression());
                while (m_current.type == Token::Type::Comma) {
                    m_current = m_lexer.next();
                    args.push_back(parseExpression());
                }
            }
            if (m_current.type == Token::Type::RightParen) {
                m_current = m_lexer.next();
            }
            return evaluateFunction(name, args);
        }

        if (m_current.type == Token::Type::Dot) {
            m_current = m_lexer.next();
            if (m_current.type == Token::Type::Identifier) {
                std::string prop = m_current.value;
                m_current = m_lexer.next();
                std::string fullName = name + "." + prop;
                if (m_current.type == Token::Type::Dot) {
                    m_current = m_lexer.next();
                    if (m_current.type == Token::Type::Identifier) {
                        fullName += "." + m_current.value;
                        m_current = m_lexer.next();
                    }
                }
                return getBuiltInVariable(fullName);
            }
            return getBuiltInVariable(name);
        }

        return getBuiltInVariable(name);
    }

    if (m_current.type == Token::Type::String) {
        std::string val = m_current.value;
        m_current = m_lexer.next();
        try {
            return std::stod(val);
        } catch (...) {
            return 0.0;
        }
    }

    double val = m_current.numValue;
    m_current = m_lexer.next();
    return val;
}

std::string ExpressionEngine::Parser::parseStringPrimary() {
    if (m_current.type == Token::Type::String) {
        std::string val = m_current.value;
        m_current = m_lexer.next();
        return val;
    }
    if (m_current.type == Token::Type::LeftParen) {
        m_current = m_lexer.next();
        std::string val = parseStringExpression();
        if (m_current.type == Token::Type::RightParen) {
            m_current = m_lexer.next();
        }
        return val;
    }
    double num = parseExpression();
    std::ostringstream oss;
    oss << num;
    return oss.str();
}

std::string ExpressionEngine::Parser::parseStringConcat() {
    std::string result = parseStringPrimary();
    while (m_current.type == Token::Type::Plus) {
        Token saved = m_current;
        m_current = m_lexer.next();
        if (m_current.type == Token::Type::String || m_current.type == Token::Type::LeftParen) {
            result += parseStringPrimary();
        } else {
            m_current = saved;
            break;
        }
    }
    return result;
}

double ExpressionEngine::Parser::evaluateFunction(const std::string& name, std::vector<double> args) {
    if (name == "wiggle") {
        double freq = (args.size() >= 1) ? args[0] : 1.0;
        double amp = (args.size() >= 2) ? args[1] : 1.0;
        static std::mt19937 rng((std::random_device())());
        std::normal_distribution<double> dist(0.0, 1.0);
        double t = m_context.time * freq;
        double noise = std::sin(t * 12.9898) * 43758.5453;
        noise = noise - std::floor(noise);
        double noise2 = std::sin(t * 78.233) * 43758.5453;
        noise2 = noise2 - std::floor(noise2);
        return amp * (noise * 2.0 - 1.0) * 0.5 + amp * (noise2 * 2.0 - 1.0) * 0.5;
    }

    if (name == "loopOut") {
        std::string mode = (args.size() >= 1) ? "cycle" : "cycle";
        double duration = m_context.endTime - m_context.startTime;
        if (duration <= 0.0) duration = 10.0;
        double t = std::fmod(m_context.time - m_context.startTime, duration);
        if (t < 0.0) t += duration;
        return m_context.value + t * 10.0;
    }

    if (name == "loopIn") {
        double duration = m_context.endTime - m_context.startTime;
        if (duration <= 0.0) duration = 10.0;
        double t = std::fmod(m_context.time, duration);
        return m_context.value;
    }

    if (name == "smooth") {
        double window = (args.size() >= 1) ? args[0] : 0.5;
        return m_context.value * (1.0 - window * 0.5);
    }

    if (name == "posterizeTime") {
        double framesPerSec = (args.size() >= 1) ? args[0] : 1.0;
        if (framesPerSec < 1.0) framesPerSec = 1.0;
        double step = 1.0 / framesPerSec;
        double t = std::floor(m_context.time / step) * step;
        return m_context.value;
    }

    if (name == "linear") {
        if (args.size() >= 4) {
            double t = (args[1] != args[2]) ? (args[0] - args[1]) / (args[2] - args[1]) : 0.0;
            t = std::max(0.0, std::min(1.0, t));
            return args[3] + t * (args[4 >= args.size() ? 3 : 4] - args[3]);
        }
        return 0.0;
    }

    if (name == "ease") {
        if (args.size() >= 4) {
            double t = (args[1] != args[2]) ? (args[0] - args[1]) / (args[2] - args[1]) : 0.0;
            t = std::max(0.0, std::min(1.0, t));
            t = t * t * (3.0 - 2.0 * t);
            return args[3] + t * (args.size() > 4 ? args[4] - args[3] : 0.0);
        }
        return 0.0;
    }

    auto it = m_engine.m_functions.find(name);
    if (it != m_engine.m_functions.end()) {
        return it->second(args);
    }

    return 0.0;
}

double ExpressionEngine::Parser::getBuiltInVariable(const std::string& name) {
    if (name == "time") return m_context.time;
    if (name == "value") return m_context.value;
    if (name == "index") return static_cast<double>(m_context.index);
    if (name == "numLayers") return static_cast<double>(m_context.numLayers);
    if (name == "comp.width") return m_context.compWidth;
    if (name == "comp.height") return m_context.compHeight;
    if (name == "comp.width") return m_context.compWidth;
    if (name == "fps") return m_context.fps;
    if (name == "startTime") return m_context.startTime;
    if (name == "endTime") return m_context.endTime;
    if (name == "loopCount") return static_cast<double>(m_context.loopCount);

    auto it = m_engine.m_variables.find(name);
    if (it != m_engine.m_variables.end()) {
        return it->second;
    }

    return 0.0;
}

} // namespace FreeEffect
