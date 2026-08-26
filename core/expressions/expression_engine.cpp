#include "expression_engine.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <random>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace FreeEffect {

ExpressionEngine& ExpressionEngine::instance() {
    static ExpressionEngine eng;
    return eng;
}

ExpressionEngine::ExpressionEngine() {
    registerBuiltInFunctions();
}

// ======================== Public API ========================

ExprValue ExpressionEngine::evaluate(const std::string& expression, double time,
                                     const std::unordered_map<std::string, ExprValue>& vars) {
    m_currentTime = time;
    for (auto& [k, v] : vars) {
        if (std::holds_alternative<double>(v)) m_variables[k] = std::get<double>(v);
        else if (std::holds_alternative<std::string>(v)) m_stringVariables[k] = std::get<std::string>(v);
    }
    ExpressionContext ctx;
    ctx.time = time;
    ctx.value = m_currentValue;
    ctx.index = m_layerIndex;
    Lexer lexer(expression);
    Parser parser(lexer, *this);
    ASTEvaluator evaluator(*this, ctx);
    auto node = parser.parseExpression();
    if (!node) return 0.0;
    return node->eval(evaluator);
}

double ExpressionEngine::evaluate(const std::string& expression, const ExpressionContext& context) {
    Lexer lexer(expression);
    Parser parser(lexer, *this);
    ASTEvaluator evaluator(*this, context);
    auto node = parser.parseExpression();
    if (!node) return 0.0;
    return node->eval(evaluator);
}

std::string ExpressionEngine::evaluateString(const std::string& expression, const ExpressionContext& context) {
    Lexer lexer(expression);
    Parser parser(lexer, *this);
    return parser.parseStringExpression();
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

void ExpressionEngine::registerFunction(const std::string& name,
                                         std::function<double(const std::vector<double>&)> func) {
    m_functions[name] = std::move(func);
}

// ======================== Noise ========================

double ExpressionEngine::noise1(double x) const {
    int ix = static_cast<int>(std::floor(x));
    double fx = x - ix;
    fx = fx * fx * (3.0 - 2.0 * fx);
    auto hash = [](int n) -> double {
        n = (n << 13) ^ n;
        return 1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0;
    };
    return hash(ix) * (1.0 - fx) + hash(ix + 1) * fx;
}

double ExpressionEngine::noise2D(double x, double y) const {
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    double fx = x - ix;
    double fy = y - iy;
    fx = fx * fx * (3.0 - 2.0 * fx);
    fy = fy * fy * (3.0 - 2.0 * fy);
    auto hash2 = [](int n) -> double {
        n = (n << 13) ^ n;
        return 1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0;
    };
    double v00 = hash2(ix + iy * 57);
    double v10 = hash2(ix + 1 + iy * 57);
    double v01 = hash2(ix + (iy + 1) * 57);
    double v11 = hash2(ix + 1 + (iy + 1) * 57);
    double i1 = v00 * (1.0 - fx) + v10 * fx;
    double i2 = v01 * (1.0 - fx) + v11 * fx;
    return i1 * (1.0 - fy) + i2 * fy;
}

double ExpressionEngine::multiOctaveNoise(double x, int octaves, double ampMult) const {
    double total = 0.0;
    double amplitude = 1.0;
    double frequency = 1.0;
    double maxAmp = 0.0;
    for (int i = 0; i < octaves; i++) {
        total += noise1(x * frequency) * amplitude;
        maxAmp += amplitude;
        amplitude *= ampMult;
        frequency *= 2.0;
    }
    return (maxAmp > 0.0) ? total / maxAmp : 0.0;
}

// ======================== Wiggle ========================

double ExpressionEngine::wiggleInternal(double freq, double amp, double octaves,
                                         double ampMult, int seedOffset, double t) {
    double total = 0.0;
    double amplitude = amp;
    double frequency = freq;
    for (int i = 0; i < static_cast<int>(octaves); i++) {
        double seed = static_cast<double>(seedOffset + i) * 127.1;
        double phase = noise1(seed) * 1000.0;
        double n = noise2D(t * frequency + phase, seed);
        total += (n * 2.0 - 1.0) * amplitude;
        amplitude *= ampMult;
        frequency *= 2.0;
    }
    return total;
}

double ExpressionEngine::wiggle(double freq, double amp, double octaves, double ampMult) {
    return wiggleInternal(freq, amp, octaves, ampMult, 0, m_currentTime);
}

double ExpressionEngine::wiggle3D(double freq, double amp, double octaves) {
    return wiggleInternal(freq, amp, octaves, 0.5, 100, m_currentTime);
}

double ExpressionEngine::wiggle4D(double freq, double amp, double octaves) {
    return wiggleInternal(freq, amp, octaves, 0.5, 200, m_currentTime);
}

double ExpressionEngine::wiggleFreq(double time) {
    return m_variables.count("wiggleFreq") ? m_variables["wiggleFreq"] : 1.0;
}

double ExpressionEngine::wiggleAmp(double time) {
    return m_variables.count("wiggleAmp") ? m_variables["wiggleAmp"] : 1.0;
}

// ======================== Loop ========================

double ExpressionEngine::loopOut(const std::string& type, double duration) {
    const auto& ctx = m_context;
    double t0 = ctx.startTime;
    double t1 = ctx.endTime;
    double dur = (duration > 0) ? duration : (t1 - t0);
    if (dur <= 0.0) dur = 1.0;
    double t = ctx.time - t0;
    if (type == "cycle" || type == "cycle") {
        t = std::fmod(t, dur);
        if (t < 0) t += dur;
        return ctx.value + t * 10.0;
    } else if (type == "pingpong") {
        t = std::fmod(t, dur * 2.0);
        if (t < 0) t += dur * 2.0;
        if (t > dur) t = dur * 2.0 - t;
        return ctx.value + t * 10.0;
    } else if (type == "offset") {
        int cycles = static_cast<int>(t / dur);
        t = std::fmod(t, dur);
        return ctx.value + cycles * dur * 10.0 + t * 10.0;
    } else if (type == "continue") {
        return ctx.value + t * 10.0;
    }
    return ctx.value;
}

double ExpressionEngine::loopIn(const std::string& type, double duration) {
    const auto& ctx = m_context;
    double dur = (duration > 0) ? duration : (ctx.endTime - ctx.startTime);
    if (dur <= 0.0) dur = 1.0;
    double t = ctx.time;
    if (type == "cycle") {
        t = std::fmod(t, dur);
        if (t < 0) t += dur;
    }
    return ctx.value;
}

double ExpressionEngine::loopOutDuration(const std::string& type) {
    return loopOut(type);
}

double ExpressionEngine::loopInDuration(const std::string& type) {
    return loopIn(type);
}

double ExpressionEngine::pingpong() {
    return loopOut("pingpong");
}

double ExpressionEngine::loopOffset() {
    return loopOut("offset");
}

// ======================== Interpolation ========================

double ExpressionEngine::linear(double value, double inMin, double inMax, double outMin, double outMax) {
    if (inMax == inMin) return outMin;
    double t = (value - inMin) / (inMax - inMin);
    t = std::clamp(t, 0.0, 1.0);
    return outMin + t * (outMax - outMin);
}

double ExpressionEngine::ease(double value, double inMin, double inMax, double outMin, double outMax) {
    if (inMax == inMin) return outMin;
    double t = (value - inMin) / (inMax - inMin);
    t = std::clamp(t, 0.0, 1.0);
    t = t * t * (3.0 - 2.0 * t);
    return outMin + t * (outMax - outMin);
}

double ExpressionEngine::easeIn(double value, double inMin, double inMax, double outMin, double outMax) {
    if (inMax == inMin) return outMin;
    double t = (value - inMin) / (inMax - inMin);
    t = std::clamp(t, 0.0, 1.0);
    t = t * t;
    return outMin + t * (outMax - outMin);
}

double ExpressionEngine::easeOut(double value, double inMin, double inMax, double outMin, double outMax) {
    if (inMax == inMin) return outMin;
    double t = (value - inMin) / (inMax - inMin);
    t = std::clamp(t, 0.0, 1.0);
    t = 1.0 - (1.0 - t) * (1.0 - t);
    return outMin + t * (outMax - outMin);
}

double ExpressionEngine::easeT(double t) {
    t = std::clamp(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

double ExpressionEngine::smooth(double width, double samples) {
    double halfWidth = width / 2.0;
    double t = m_currentTime;
    double sum = 0.0;
    int n = static_cast<int>(samples);
    if (n < 1) n = 1;
    for (int i = 0; i < n; i++) {
        double s = static_cast<double>(i) / static_cast<double>(n - 1);
        double st = t - halfWidth + s * width;
        sum += m_currentValue; // Simplified: returns smoothed value
    }
    return m_currentValue * (1.0 - width * 0.01);
}

// ======================== Math ========================

double ExpressionEngine::clamp(double value, double minVal, double maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

double ExpressionEngine::mapRange(double value, double low1, double high1, double low2, double high2) {
    if (high1 == low1) return low2;
    double t = (value - low1) / (high1 - low1);
    return low2 + t * (high2 - low2);
}

double ExpressionEngine::normalize(double value, double minVal, double maxVal) {
    if (maxVal == minVal) return 0.0;
    return (value - minVal) / (maxVal - minVal);
}

double ExpressionEngine::degreesToRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

double ExpressionEngine::radiansToDegrees(double radians) {
    return radians * 180.0 / M_PI;
}

double ExpressionEngine::vecToAngle(double x, double y) {
    return std::atan2(y, x) * 180.0 / M_PI;
}

double ExpressionEngine::angleToVecX(double angle) {
    return std::cos(angle * M_PI / 180.0);
}

double ExpressionEngine::angleToVecY(double angle) {
    return std::sin(angle * M_PI / 180.0);
}

// ======================== Vector math ========================

std::vector<double> ExpressionEngine::addVec(const std::vector<double>& a, const std::vector<double>& b) {
    size_t n = std::min(a.size(), b.size());
    std::vector<double> r(n);
    for (size_t i = 0; i < n; i++) r[i] = a[i] + b[i];
    return r;
}

std::vector<double> ExpressionEngine::subVec(const std::vector<double>& a, const std::vector<double>& b) {
    size_t n = std::min(a.size(), b.size());
    std::vector<double> r(n);
    for (size_t i = 0; i < n; i++) r[i] = a[i] - b[i];
    return r;
}

std::vector<double> ExpressionEngine::mulVec(const std::vector<double>& a, double s) {
    std::vector<double> r(a.size());
    for (size_t i = 0; i < a.size(); i++) r[i] = a[i] * s;
    return r;
}

std::vector<double> ExpressionEngine::divVec(const std::vector<double>& a, double s) {
    std::vector<double> r(a.size());
    if (s != 0.0) for (size_t i = 0; i < a.size(); i++) r[i] = a[i] / s;
    return r;
}

double ExpressionEngine::dotVec(const std::vector<double>& a, const std::vector<double>& b) {
    size_t n = std::min(a.size(), b.size());
    double r = 0.0;
    for (size_t i = 0; i < n; i++) r += a[i] * b[i];
    return r;
}

std::vector<double> ExpressionEngine::crossVec(const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() >= 3 && b.size() >= 3) {
        return {a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0]};
    }
    return {0.0};
}

double ExpressionEngine::vecLength(const std::vector<double>& v) {
    double s = 0.0;
    for (double x : v) s += x * x;
    return std::sqrt(s);
}

std::vector<double> ExpressionEngine::normalizeVec(const std::vector<double>& v) {
    double len = vecLength(v);
    if (len < 1e-12) return v;
    std::vector<double> r(v.size());
    for (size_t i = 0; i < v.size(); i++) r[i] = v[i] / len;
    return r;
}

std::vector<double> ExpressionEngine::interpolateVectors(const std::vector<double>& a,
                                                         const std::vector<double>& b, double t) {
    size_t n = std::min(a.size(), b.size());
    std::vector<double> r(n);
    for (size_t i = 0; i < n; i++) r[i] = a[i] + (b[i] - a[i]) * t;
    return r;
}

// ======================== Random ========================

double ExpressionEngine::random(double max) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, max);
    return dist(rng);
}

double ExpressionEngine::random2(double min, double max) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

double ExpressionEngine::seedRandom(double seed) {
    auto it = m_seedCache.find(static_cast<int>(seed));
    if (it != m_seedCache.end()) return it->second;
    double v = std::fmod(std::sin(seed * 12.9898 + 78.233) * 43758.5453, 1.0);
    if (v < 0) v += 1.0;
    m_seedCache[static_cast<int>(seed)] = v;
    return v;
}

double ExpressionEngine::gaussRandom(double mean, double stdev) {
    static std::mt19937 rng(std::random_device{}());
    std::normal_distribution<double> dist(mean, stdev);
    return dist(rng);
}

// ======================== Layer/Comp references ========================

double ExpressionEngine::layerIndex(const std::string& name) {
    auto it = m_layerData.find(name + ".index");
    return (it != m_layerData.end()) ? it->second : 1.0;
}

double ExpressionEngine::layerInPoint(const std::string& name) {
    auto it = m_layerData.find(name + ".inPoint");
    return (it != m_layerData.end()) ? it->second : 0.0;
}

double ExpressionEngine::layerOutPoint(const std::string& name) {
    auto it = m_layerData.find(name + ".outPoint");
    return (it != m_layerData.end()) ? it->second : 10.0;
}

double ExpressionEngine::layerDuration(const std::string& name) {
    return layerOutPoint(name) - layerInPoint(name);
}

std::vector<double> ExpressionEngine::layerPosition(const std::string& name, double t) {
    auto it = m_layerData.find(name + ".position.x");
    double x = (it != m_layerData.end()) ? it->second : 960.0;
    auto it2 = m_layerData.find(name + ".position.y");
    double y = (it2 != m_layerData.end()) ? it2->second : 540.0;
    return {x, y};
}

double ExpressionEngine::layerEffectValue(const std::string& layerName, const std::string& effectName,
                                          const std::string& paramName, double t) {
    std::string key = layerName + ".effect." + effectName + "." + paramName;
    auto it = m_layerData.find(key);
    return (it != m_layerData.end()) ? it->second : 0.0;
}

// ======================== sourceRect ========================

ExpressionEngine::SourceRect ExpressionEngine::sourceRectAtTime(double time) {
    return {0.0, 0.0, 1920.0, 1080.0};
}

// ======================== Keyframe access ========================

double ExpressionEngine::key(const std::string& property, int index) {
    auto it = m_keyframeData.find(property);
    if (it == m_keyframeData.end()) return 0.0;
    if (index < 1 || index > static_cast<int>(it->second.size())) return 0.0;
    return it->second[index - 1].second;
}

int ExpressionEngine::numKeys(const std::string& property) {
    auto it = m_keyframeData.find(property);
    return (it != m_keyframeData.end()) ? static_cast<int>(it->second.size()) : 0;
}

// ======================== Lexer ========================

ExpressionEngine::Lexer::Lexer(const std::string& source) : m_source(source), m_pos(0) {}

void ExpressionEngine::Lexer::skipWhitespace() {
    while (m_pos < m_source.size() &&
           (m_source[m_pos] == ' ' || m_source[m_pos] == '\t' ||
            m_source[m_pos] == '\r' || m_source[m_pos] == '\n')) {
        m_pos++;
    }
    // skip line comments
    if (m_pos + 1 < m_source.size() && m_source[m_pos] == '/' && m_source[m_pos + 1] == '/') {
        while (m_pos < m_source.size() && m_source[m_pos] != '\n') m_pos++;
        skipWhitespace();
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
    // handle scientific notation
    if (m_pos < m_source.size() && (m_source[m_pos] == 'e' || m_source[m_pos] == 'E')) {
        m_pos++;
        if (m_pos < m_source.size() && (m_source[m_pos] == '+' || m_source[m_pos] == '-')) m_pos++;
        while (m_pos < m_source.size() && std::isdigit(m_source[m_pos])) m_pos++;
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
    m_pos++; // skip opening quote
    size_t start = m_pos;
    while (m_pos < m_source.size() && m_source[m_pos] != '"') {
        if (m_source[m_pos] == '\\') m_pos++;
        m_pos++;
    }
    tok.value = m_source.substr(start, m_pos - start);
    if (m_pos < m_source.size()) m_pos++; // skip closing quote
    return tok;
}

void ExpressionEngine::Lexer::advance() {
    if (!m_peeked.empty()) m_peeked.erase(m_peeked.begin());
}

ExpressionEngine::Token ExpressionEngine::Lexer::peek() {
    if (!m_peeked.empty()) return m_peeked.back();
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
    if (std::isalpha(c) || c == '_') return readIdentifier();
    if (c == '"') return readString();

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
        case ';': tok.type = Token::Type::Semicolon; tok.value = ";"; break;
        case '=':
            if (m_pos < m_source.size() && m_source[m_pos] == '=') {
                m_pos++;
                tok.type = Token::Type::EqualsEquals; tok.value = "==";
            } else {
                tok.type = Token::Type::Equals; tok.value = "=";
            }
            break;
        case '!':
            if (m_pos < m_source.size() && m_source[m_pos] == '=') {
                m_pos++;
                tok.type = Token::Type::NotEquals; tok.value = "!=";
            } else {
                tok.type = Token::Type::Not; tok.value = "!";
            }
            break;
        case '<':
            if (m_pos < m_source.size() && m_source[m_pos] == '=') {
                m_pos++;
                tok.type = Token::Type::LessEqual; tok.value = "<=";
            } else {
                tok.type = Token::Type::Less; tok.value = "<";
            }
            break;
        case '>':
            if (m_pos < m_source.size() && m_source[m_pos] == '=') {
                m_pos++;
                tok.type = Token::Type::GreaterEqual; tok.value = ">=";
            } else {
                tok.type = Token::Type::Greater; tok.value = ">";
            }
            break;
        case '&':
            if (m_pos < m_source.size() && m_source[m_pos] == '&') {
                m_pos++;
                tok.type = Token::Type::And; tok.value = "&&";
            }
            break;
        case '|':
            if (m_pos < m_source.size() && m_source[m_pos] == '|') {
                m_pos++;
                tok.type = Token::Type::Or; tok.value = "||";
            }
            break;
        default:
            tok.type = Token::Type::Eof;
            break;
    }
    return tok;
}

// ======================== Parser ========================

ExpressionEngine::Parser::Parser(Lexer& lexer, ExpressionEngine& engine)
    : m_lexer(lexer), m_engine(engine) {
    m_current = m_lexer.next();
}

ExpressionEngine::ASTNodePtr ExpressionEngine::Parser::parseExpression() {
    ASTNodePtr left = parseTernary();
    // Handle comma expressions
    while (m_current.type == Token::Type::Comma) {
        m_current = m_lexer.next();
        ASTNodePtr right = parseTernary();
        std::vector<ASTNodePtr> exprs;
        exprs.push_back(std::move(left));
        exprs.push_back(std::move(right));
        left = std::make_unique<CommaExprNode>(std::move(exprs));
    }
    return left;
}

std::string ExpressionEngine::Parser::parseStringExpression() {
    return parseStringConcat();
}

ExpressionEngine::ASTNodePtr ExpressionEngine::Parser::parseTernary() {
    ASTNodePtr cond = parseOr();
    if (m_current.type == Token::Type::Question) {
        m_current = m_lexer.next();
        ASTNodePtr trueExpr = parseExpression();
        if (m_current.type == Token::Type::Colon) m_current = m_lexer.next();
        ASTNodePtr falseExpr = parseExpression();
        return std::make_unique<TernaryNode>(std::move(cond), std::move(trueExpr), std::move(falseExpr));
    }
    return cond;
}

ExpressionEngine::ASTNodePtr ExpressionEngine::Parser::parseOr() {
    ASTNodePtr left = parseAnd();
    while (m_current.type == Token::Type::Or) {
        m_current = m_lexer.next();
        ASTNodePtr right = parseAnd();
        left = std::make_unique<BinaryOpNode>(BinaryOpNode::Or, std::move(left), std::move(right));
    }
    return left;
}

ExpressionEngine::ASTNodePtr ExpressionEngine::Parser::parseAnd() {
    ASTNodePtr left = parseComparison();
    while (m_current.type == Token::Type::And) {
        m_current = m_lexer.next();
        ASTNodePtr right = parseComparison();
        left = std::make_unique<BinaryOpNode>(BinaryOpNode::And, std::move(left), std::move(right));
    }
    return left;
}

ExpressionEngine::ASTNodePtr ExpressionEngine::Parser::parseComparison() {
    ASTNodePtr left = parseAddSub();
    while (m_current.type == Token::Type::EqualsEquals || m_current.type == Token::Type::NotEquals ||
           m_current.type == Token::Type::Less || m_current.type == Token::Type::LessEqual ||
           m_current.type == Token::Type::Greater || m_current.type == Token::Type::GreaterEqual) {
        BinaryOpNode::Op op;
        switch (m_current.type) {
            case Token::Type::EqualsEquals: op = BinaryOpNode::Eq; break;
            case Token::Type::NotEquals: op = BinaryOpNode::Neq; break;
            case Token::Type::Less: op = BinaryOpNode::Lt; break;
            case Token::Type::LessEqual: op = BinaryOpNode::Lte; break;
            case Token::Type::Greater: op = BinaryOpNode::Gt; break;
            case Token::Type::GreaterEqual: op = BinaryOpNode::Gte; break;
            default: op = BinaryOpNode::Eq; break;
        }
        m_current = m_lexer.next();
        ASTNodePtr right = parseAddSub();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

ExpressionEngine::ASTNodePtr ExpressionEngine::Parser::parseAddSub() {
    ASTNodePtr left = parseMulDiv();
    while (m_current.type == Token::Type::Plus || m_current.type == Token::Type::Minus) {
        bool isAdd = (m_current.type == Token::Type::Plus);
        m_current = m_lexer.next();
        ASTNodePtr right = parseMulDiv();
        left = std::make_unique<BinaryOpNode>(isAdd ? BinaryOpNode::Add : BinaryOpNode::Sub,
                                              std::move(left), std::move(right));
    }
    return left;
}

ExpressionEngine::ASTNodePtr ExpressionEngine::Parser::parseMulDiv() {
    ASTNodePtr left = parseUnary();
    while (m_current.type == Token::Type::Star || m_current.type == Token::Type::Slash ||
           m_current.type == Token::Type::Percent) {
        BinaryOpNode::Op op;
        switch (m_current.type) {
            case Token::Type::Star: op = BinaryOpNode::Mul; break;
            case Token::Type::Slash: op = BinaryOpNode::Div; break;
            case Token::Type::Percent: op = BinaryOpNode::Mod; break;
            default: op = BinaryOpNode::Mul; break;
        }
        m_current = m_lexer.next();
        ASTNodePtr right = parseUnary();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

ExpressionEngine::ASTNodePtr ExpressionEngine::Parser::parseUnary() {
    if (m_current.type == Token::Type::Minus) {
        m_current = m_lexer.next();
        return std::make_unique<UnaryOpNode>(UnaryOpNode::Neg, parseUnary());
    }
    if (m_current.type == Token::Type::Plus) {
        m_current = m_lexer.next();
        return std::make_unique<UnaryOpNode>(UnaryOpNode::Positive, parseUnary());
    }
    if (m_current.type == Token::Type::Not) {
        m_current = m_lexer.next();
        return std::make_unique<UnaryOpNode>(UnaryOpNode::Not, parseUnary());
    }
    return parsePrimary();
}

ExpressionEngine::ASTNodePtr ExpressionEngine::Parser::parsePrimary() {
    // Number literal
    if (m_current.type == Token::Type::Number) {
        double val = m_current.numValue;
        m_current = m_lexer.next();
        return std::make_unique<NumberNode>(val);
    }

    // Parenthesized expression
    if (m_current.type == Token::Type::LeftParen) {
        m_current = m_lexer.next();
        auto expr = parseExpression();
        if (m_current.type == Token::Type::RightParen) m_current = m_lexer.next();
        return expr;
    }

    // Array literal [a, b, c]
    if (m_current.type == Token::Type::LeftBracket) {
        m_current = m_lexer.next();
        std::vector<double> vals;
        if (m_current.type != Token::Type::RightBracket) {
            auto first = parseExpression();
            // For simplicity in array context, evaluate immediately if simple values
            vals.push_back(0.0); // placeholder
        }
        if (m_current.type == Token::Type::RightBracket) m_current = m_lexer.next();
        // Simplified: return first element as scalar
        return std::make_unique<NumberNode>(vals.empty() ? 0.0 : vals[0]);
    }

    // Identifier: variable, function call, or member access
    if (m_current.type == Token::Type::Identifier) {
        std::string name = m_current.value;
        m_current = m_lexer.next();

        // Function call
        if (m_current.type == Token::Type::LeftParen) {
            m_current = m_lexer.next();
            std::vector<ASTNodePtr> args;
            if (m_current.type != Token::Type::RightParen) {
                args.push_back(parseExpression());
                while (m_current.type == Token::Type::Comma) {
                    m_current = m_lexer.next();
                    args.push_back(parseExpression());
                }
            }
            if (m_current.type == Token::Type::RightParen) m_current = m_lexer.next();
            return std::make_unique<FunctionCallNode>(name, std::move(args));
        }

        // Member access: thisLayer.transform.position or comp.width
        if (m_current.type == Token::Type::Dot) {
            m_current = m_lexer.next();
            if (m_current.type == Token::Type::Identifier) {
                std::string prop = m_current.value;
                m_current = m_lexer.next();
                auto node = std::make_unique<MemberAccessNode>(name, prop);
                if (m_current.type == Token::Type::Dot) {
                    m_current = m_lexer.next();
                    if (m_current.type == Token::Type::Identifier) {
                        node->setSubProperty(m_current.value);
                        m_current = m_lexer.next();
                    }
                }
                return node;
            }
            return std::make_unique<VariableNode>(name);
        }

        // Assignment: time = expr (shouldn't normally happen, but handle = as ==)
        if (m_current.type == Token::Type::Equals) {
            m_current = m_lexer.next();
            auto val = parseExpression();
            return std::make_unique<AssignNode>(name, std::move(val));
        }

        return std::make_unique<VariableNode>(name);
    }

    // String literal
    if (m_current.type == Token::Type::String) {
        std::string val = m_current.value;
        m_current = m_lexer.next();
        return std::make_unique<StringNode>(val);
    }

    // Fallback
    m_current = m_lexer.next();
    return std::make_unique<NumberNode>(0.0);
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
        if (m_current.type == Token::Type::RightParen) m_current = m_lexer.next();
        return val;
    }
    auto node = parseExpression();
    ExpressionContext ctx;
    ASTEvaluator eval(m_engine, ctx);
    double num = node->eval(eval);
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

// ======================== AST Evaluator ========================

double ExpressionEngine::ASTEvaluator::getBuiltInVariable(const std::string& name) {
    // Built-in constants
    if (name == "time") return m_context.time;
    if (name == "value") return m_context.value;
    if (name == "index") return static_cast<double>(m_context.index);
    if (name == "numLayers") return static_cast<double>(m_context.numLayers);
    if (name == "comp.width" || name == "thisComp.width") return m_context.compWidth;
    if (name == "comp.height" || name == "thisComp.height") return m_context.compHeight;
    if (name == "fps") return m_context.fps;
    if (name == "startTime") return m_context.startTime;
    if (name == "endTime") return m_context.endTime;
    if (name == "loopCount") return static_cast<double>(m_context.loopCount);
    if (name == "PI" || name == "Math.PI") return M_PI;
    if (name == "true") return 1.0;
    if (name == "false") return 0.0;

    // Check user variables
    auto it = m_engine.m_variables.find(name);
    if (it != m_engine.m_variables.end()) return it->second;

    // Check layer data
    auto lit = m_engine.m_layerData.find(name);
    if (lit != m_engine.m_layerData.end()) return lit->second;

    return 0.0;
}

double ExpressionEngine::ASTEvaluator::evaluateFunction(const std::string& name, const std::vector<double>& args) {
    // --- Wiggle ---
    if (name == "wiggle") {
        double freq = args.size() >= 1 ? args[0] : 1.0;
        double amp = args.size() >= 2 ? args[1] : 1.0;
        double octaves = args.size() >= 3 ? args[2] : 1.0;
        double ampMult = args.size() >= 4 ? args[3] : 0.5;
        return m_engine.wiggle(freq, amp, octaves, ampMult);
    }
    if (name == "wiggle3D") {
        return m_engine.wiggle3D(args.size() >= 1 ? args[0] : 1.0, args.size() >= 2 ? args[1] : 1.0,
                                  args.size() >= 3 ? args[2] : 1.0);
    }
    if (name == "wiggle4D") {
        return m_engine.wiggle4D(args.size() >= 1 ? args[0] : 1.0, args.size() >= 2 ? args[1] : 1.0,
                                  args.size() >= 3 ? args[2] : 1.0);
    }

    // --- Loop ---
    if (name == "loopOut") {
        std::string type = (args.size() >= 1) ? "cycle" : "cycle";
        double dur = args.size() >= 2 ? args[1] : 0.0;
        return m_engine.loopOut(type, dur);
    }
    if (name == "loopIn") {
        std::string type = (args.size() >= 1) ? "cycle" : "cycle";
        double dur = args.size() >= 2 ? args[1] : 0.0;
        return m_engine.loopIn(type, dur);
    }
    if (name == "loopOutDuration") {
        std::string type = (args.size() >= 1) ? "cycle" : "cycle";
        return m_engine.loopOutDuration(type);
    }
    if (name == "loopInDuration") {
        std::string type = (args.size() >= 1) ? "cycle" : "cycle";
        return m_engine.loopInDuration(type);
    }
    if (name == "pingpong") return m_engine.pingpong();
    if (name == "offset") return m_engine.loopOffset();

    // --- Interpolation ---
    if (name == "linear" && args.size() >= 5) {
        return m_engine.linear(args[0], args[1], args[2], args[3], args[4]);
    }
    if (name == "ease" && args.size() >= 5) {
        return m_engine.ease(args[0], args[1], args[2], args[3], args[4]);
    }
    if (name == "easeIn" && args.size() >= 5) {
        return m_engine.easeIn(args[0], args[1], args[2], args[3], args[4]);
    }
    if (name == "easeOut" && args.size() >= 5) {
        return m_engine.easeOut(args[0], args[1], args[2], args[3], args[4]);
    }
    if (name == "smooth") {
        double w = args.size() >= 1 ? args[0] : 0.5;
        double s = args.size() >= 2 ? args[1] : 5.0;
        return m_engine.smooth(w, s);
    }

    // --- Math ---
    if (name == "clamp" && args.size() >= 3) {
        return m_engine.clamp(args[0], args[1], args[2]);
    }
    if (name == "map" || name == "remap") {
        if (args.size() >= 5) return m_engine.mapRange(args[0], args[1], args[2], args[3], args[4]);
        return 0.0;
    }
    if (name == "normalize" && args.size() >= 3) {
        return m_engine.normalize(args[0], args[1], args[2]);
    }
    if (name == "degToRad" && args.size() >= 1) return m_engine.degreesToRadians(args[0]);
    if (name == "radToDeg" && args.size() >= 1) return m_engine.radiansToDegrees(args[0]);
    if (name == "vecToAngle" && args.size() >= 2) return m_engine.vecToAngle(args[0], args[1]);
    if (name == "angleToVecX" && args.size() >= 1) return m_engine.angleToVecX(args[0]);
    if (name == "angleToVecY" && args.size() >= 1) return m_engine.angleToVecY(args[0]);

    // --- Vector math ---
    if (name == "length" && args.size() >= 2) {
        if (args.size() >= 3) return std::sqrt(args[0]*args[0] + args[1]*args[1] + args[2]*args[2]);
        return std::sqrt(args[0]*args[0] + args[1]*args[1]);
    }
    if (name == "dot" && args.size() >= 4) {
        if (args.size() >= 6) return args[0]*args[3] + args[1]*args[4] + args[2]*args[5];
        return args[0]*args[2] + args[1]*args[3];
    }
    if (name == "cross" && args.size() >= 6) {
        return args[0]*args[3] + args[1]*args[4] + args[2]*args[5]; // simplified
    }

    // --- Random ---
    if (name == "random") {
        if (args.size() >= 2) return m_engine.random2(args[0], args[1]);
        if (args.size() >= 1) return m_engine.random(args[0]);
        return m_engine.random(1.0);
    }
    if (name == "seedRandom" && args.size() >= 1) return m_engine.seedRandom(args[0]);
    if (name == "gaussRandom") {
        double mean = args.size() >= 1 ? args[0] : 0.0;
        double stdev = args.size() >= 2 ? args[1] : 1.0;
        return m_engine.gaussRandom(mean, stdev);
    }

    // --- Layer ---
    if (name == "layerIndex" && args.size() >= 1) return m_engine.layerIndex(std::to_string(static_cast<int>(args[0])));
    if (name == "layerInPoint" && args.size() >= 1) return m_engine.layerInPoint(std::to_string(static_cast<int>(args[0])));
    if (name == "layerOutPoint" && args.size() >= 1) return m_engine.layerOutPoint(std::to_string(static_cast<int>(args[0])));
    if (name == "layerDuration" && args.size() >= 1) return m_engine.layerDuration(std::to_string(static_cast<int>(args[0])));

    // --- Time ---
    if (name == "posterizeTime" && args.size() >= 1) {
        double fps = args[0];
        if (fps < 1.0) fps = 1.0;
        double step = 1.0 / fps;
        return std::floor(m_context.time / step) * step;
    }

    // --- Registered functions ---
    auto it = m_engine.m_functions.find(name);
    if (it != m_engine.m_functions.end()) return it->second(args);

    // --- Standard math fallback ---
    if (name == "sin" && args.size() >= 1) return std::sin(args[0]);
    if (name == "cos" && args.size() >= 1) return std::cos(args[0]);
    if (name == "tan" && args.size() >= 1) return std::tan(args[0]);
    if (name == "asin" && args.size() >= 1) return std::asin(args[0]);
    if (name == "acos" && args.size() >= 1) return std::acos(args[0]);
    if (name == "atan" && args.size() >= 1) return std::atan(args[0]);
    if (name == "atan2" && args.size() >= 2) return std::atan2(args[0], args[1]);
    if (name == "abs" && args.size() >= 1) return std::abs(args[0]);
    if (name == "floor" && args.size() >= 1) return std::floor(args[0]);
    if (name == "ceil" && args.size() >= 1) return std::ceil(args[0]);
    if (name == "round" && args.size() >= 1) return std::round(args[0]);
    if (name == "frac" && args.size() >= 1) { double f; return std::modf(args[0], &f); }
    if (name == "sign" && args.size() >= 1) {
        return (args[0] > 0.0) ? 1.0 : ((args[0] < 0.0) ? -1.0 : 0.0);
    }
    if (name == "pow" && args.size() >= 2) return std::pow(args[0], args[1]);
    if (name == "sqrt" && args.size() >= 1) return std::sqrt(args[0]);
    if (name == "exp" && args.size() >= 1) return std::exp(args[0]);
    if (name == "log" && args.size() >= 1) return std::log(args[0]);
    if (name == "log2" && args.size() >= 1) return std::log2(args[0]);
    if (name == "min" && args.size() >= 2) return std::min(args[0], args[1]);
    if (name == "max" && args.size() >= 2) return std::max(args[0], args[1]);
    if (name == "mod" && args.size() >= 2) return std::fmod(args[0], args[1]);

    return 0.0;
}

// ======================== Built-in function registration ========================

void ExpressionEngine::registerBuiltInFunctions() {
    m_functions["sin"] = [](const std::vector<double>& a) { return a.size() >= 1 ? std::sin(a[0]) : 0.0; };
    m_functions["cos"] = [](const std::vector<double>& a) { return a.size() >= 1 ? std::cos(a[0]) : 0.0; };
    m_functions["tan"] = [](const std::vector<double>& a) { return a.size() >= 1 ? std::tan(a[0]) : 0.0; };
    m_functions["abs"] = [](const std::vector<double>& a) { return a.size() >= 1 ? std::abs(a[0]) : 0.0; };
    m_functions["floor"] = [](const std::vector<double>& a) { return a.size() >= 1 ? std::floor(a[0]) : 0.0; };
    m_functions["ceil"] = [](const std::vector<double>& a) { return a.size() >= 1 ? std::ceil(a[0]) : 0.0; };
    m_functions["round"] = [](const std::vector<double>& a) { return a.size() >= 1 ? std::round(a[0]) : 0.0; };
    m_functions["sqrt"] = [](const std::vector<double>& a) { return a.size() >= 1 ? std::sqrt(a[0]) : 0.0; };
    m_functions["pow"] = [](const std::vector<double>& a) { return a.size() >= 2 ? std::pow(a[0], a[1]) : 0.0; };
    m_functions["exp"] = [](const std::vector<double>& a) { return a.size() >= 1 ? std::exp(a[0]) : 0.0; };
    m_functions["log"] = [](const std::vector<double>& a) { return a.size() >= 1 ? std::log(a[0]) : 0.0; };
    m_functions["min"] = [](const std::vector<double>& a) { return a.size() >= 2 ? std::min(a[0], a[1]) : (a.size() >= 1 ? a[0] : 0.0); };
    m_functions["max"] = [](const std::vector<double>& a) { return a.size() >= 2 ? std::max(a[0], a[1]) : (a.size() >= 1 ? a[0] : 0.0); };
    m_functions["clamp"] = [](const std::vector<double>& a) {
        if (a.size() >= 3) { return std::max(a[1], std::min(a[2], a[0])); }
        return a.size() >= 1 ? a[0] : 0.0;
    };
    m_functions["noise"] = [this](const std::vector<double>& a) {
        return a.size() >= 1 ? noise1(a[0]) : 0.0;
    };
    m_functions["seedRandom"] = [this](const std::vector<double>& a) {
        return a.size() >= 1 ? seedRandom(a[0]) : 0.0;
    };
    m_functions["random"] = [this](const std::vector<double>& a) {
        if (a.size() >= 2) return random2(a[0], a[1]);
        if (a.size() >= 1) return random(a[0]);
        return random(1.0);
    };
    m_functions["gaussRandom"] = [this](const std::vector<double>& a) {
        double mean = a.size() >= 1 ? a[0] : 0.0;
        double sd = a.size() >= 2 ? a[1] : 1.0;
        return gaussRandom(mean, sd);
    };
    m_functions["degToRad"] = [](const std::vector<double>& a) { return a.size() >= 1 ? a[0] * M_PI / 180.0 : 0.0; };
    m_functions["radToDeg"] = [](const std::vector<double>& a) { return a.size() >= 1 ? a[0] * 180.0 / M_PI : 0.0; };
    m_functions["length"] = [](const std::vector<double>& a) {
        if (a.size() >= 3) return std::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
        if (a.size() >= 2) return std::sqrt(a[0]*a[0] + a[1]*a[1]);
        return 0.0;
    };
    m_functions["dot"] = [](const std::vector<double>& a) {
        if (a.size() >= 6) return a[0]*a[3] + a[1]*a[4] + a[2]*a[5];
        if (a.size() >= 4) return a[0]*a[2] + a[1]*a[3];
        return 0.0;
    };
}

} // namespace FreeEffect
