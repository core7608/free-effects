#include "extend_script_engine.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <cctype>

namespace FreeEffect {

ExtendScriptEngine& ExtendScriptEngine::instance() {
    static ExtendScriptEngine inst;
    return inst;
}

// ─── Tokenizer ───────────────────────────────────────────────────────

ExtendScriptEngine::Tokenizer::Tokenizer(const std::string& src) : m_src(src), m_pos(0) {}

void ExtendScriptEngine::Tokenizer::skipWhitespace() {
    while (m_pos < m_src.size()) {
        char c = m_src[m_pos];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            m_pos++;
        } else if (c == '/' && m_pos + 1 < m_src.size() && m_src[m_pos + 1] == '/') {
            while (m_pos < m_src.size() && m_src[m_pos] != '\n') m_pos++;
        } else if (c == '/' && m_pos + 1 < m_src.size() && m_src[m_pos + 1] == '*') {
            m_pos += 2;
            while (m_pos + 1 < m_src.size() && !(m_src[m_pos] == '*' && m_src[m_pos + 1] == '/')) m_pos++;
            if (m_pos + 1 < m_src.size()) m_pos += 2;
        } else {
            break;
        }
    }
}

ExtendScriptEngine::Token ExtendScriptEngine::Tokenizer::readNumber() {
    Token tok;
    tok.type = TokenType::Number;
    size_t start = m_pos;
    while (m_pos < m_src.size() && (std::isdigit(m_src[m_pos]) || m_src[m_pos] == '.')) m_pos++;
    tok.value = m_src.substr(start, m_pos - start);
    tok.numValue = std::stod(tok.value);
    return tok;
}

ExtendScriptEngine::Token ExtendScriptEngine::Tokenizer::readString() {
    Token tok;
    tok.type = TokenType::String;
    char quote = m_src[m_pos++];
    size_t start = m_pos;
    while (m_pos < m_src.size() && m_src[m_pos] != quote) {
        if (m_src[m_pos] == '\\') m_pos++;
        m_pos++;
    }
    tok.value = m_src.substr(start, m_pos - start);
    if (m_pos < m_src.size()) m_pos++;
    return tok;
}

ExtendScriptEngine::Token ExtendScriptEngine::Tokenizer::readIdentifier() {
    Token tok;
    tok.type = TokenType::Identifier;
    size_t start = m_pos;
    while (m_pos < m_src.size() && (std::isalnum(m_src[m_pos]) || m_src[m_pos] == '_' || m_src[m_pos] == '$')) m_pos++;
    tok.value = m_src.substr(start, m_pos - start);
    return tok;
}

ExtendScriptEngine::Token ExtendScriptEngine::Tokenizer::next() {
    if (!m_peeked.empty()) {
        Token t = m_peeked.back();
        m_peeked.pop_back();
        return t;
    }
    skipWhitespace();
    if (m_pos >= m_src.size()) {
        Token tok; tok.type = TokenType::Eof; return tok;
    }
    char c = m_src[m_pos];
    if (std::isdigit(c)) return readNumber();
    if (c == '"' || c == '\'') return readString();
    if (std::isalpha(c) || c == '_' || c == '$') return readIdentifier();

    Token tok;
    tok.type = TokenType::Eof;
    tok.value = std::string(1, c);
    m_pos++;

    switch (c) {
        case '+': tok.type = TokenType::Plus; break;
        case '-': tok.type = TokenType::Minus; break;
        case '*': tok.type = TokenType::Star; break;
        case '/': tok.type = TokenType::Slash; break;
        case '%': tok.type = TokenType::Percent; break;
        case '(': tok.type = TokenType::LeftParen; break;
        case ')': tok.type = TokenType::RightParen; break;
        case '[': tok.type = TokenType::LeftBracket; break;
        case ']': tok.type = TokenType::RightBracket; break;
        case ',': tok.type = TokenType::Comma; break;
        case '.': tok.type = TokenType::Dot; break;
        case ';': tok.type = TokenType::Semicolon; break;
        case '=':
            if (m_pos < m_src.size() && m_src[m_pos] == '=') {
                tok.type = TokenType::EqualsEquals; m_pos++;
            } else {
                tok.type = TokenType::Equals;
            }
            break;
        case '!':
            if (m_pos < m_src.size() && m_src[m_pos] == '=') {
                tok.type = TokenType::NotEquals; m_pos++;
            } else {
                tok.type = TokenType::Not;
            }
            break;
        case '<':
            if (m_pos < m_src.size() && m_src[m_pos] == '=') {
                tok.type = TokenType::LessEqual; m_pos++;
            } else {
                tok.type = TokenType::Less;
            }
            break;
        case '>':
            if (m_pos < m_src.size() && m_src[m_pos] == '=') {
                tok.type = TokenType::GreaterEqual; m_pos++;
            } else {
                tok.type = TokenType::Greater;
            }
            break;
        case '&':
            if (m_pos < m_src.size() && m_src[m_pos] == '&') {
                tok.type = TokenType::And; m_pos++;
            }
            break;
        case '|':
            if (m_pos < m_src.size() && m_src[m_pos] == '|') {
                tok.type = TokenType::Or; m_pos++;
            }
            break;
    }
    return tok;
}

ExtendScriptEngine::Token ExtendScriptEngine::Tokenizer::peek() {
    Token t = next();
    m_peeked.push_back(t);
    return t;
}

// ─── Parser (recursive descent) ──────────────────────────────────────

ScriptValue ExtendScriptEngine::parseExpression(Tokenizer& tok) {
    return parseOr(tok);
}

ScriptValue ExtendScriptEngine::parseOr(Tokenizer& tok) {
    auto left = parseAnd(tok);
    while (tok.peek().type == TokenType::Or) {
        tok.next();
        auto right = parseAnd(tok);
        bool lv = std::holds_alternative<bool>(left) ? std::get<bool>(left) :
                  (std::holds_alternative<double>(left) ? std::get<double>(left) != 0 : !std::get<std::string>(left).empty());
        bool rv = std::holds_alternative<bool>(right) ? std::get<bool>(right) :
                  (std::holds_alternative<double>(right) ? std::get<double>(right) != 0 : !std::get<std::string>(right).empty());
        left = ScriptValue(lv || rv);
    }
    return left;
}

ScriptValue ExtendScriptEngine::parseAnd(Tokenizer& tok) {
    auto left = parseComparison(tok);
    while (tok.peek().type == TokenType::And) {
        tok.next();
        auto right = parseComparison(tok);
        bool lv = std::holds_alternative<bool>(left) ? std::get<bool>(left) :
                  (std::holds_alternative<double>(left) ? std::get<double>(left) != 0 : !std::get<std::string>(left).empty());
        bool rv = std::holds_alternative<bool>(right) ? std::get<bool>(right) :
                  (std::holds_alternative<double>(right) ? std::get<double>(right) != 0 : !std::get<std::string>(right).empty());
        left = ScriptValue(lv && rv);
    }
    return left;
}

ScriptValue ExtendScriptEngine::parseComparison(Tokenizer& tok) {
    auto left = parseAddSub(tok);
    Token t = tok.peek();
    if (t.type == TokenType::EqualsEquals || t.type == TokenType::NotEquals ||
        t.type == TokenType::Less || t.type == TokenType::LessEqual ||
        t.type == TokenType::Greater || t.type == TokenType::GreaterEqual) {
        tok.next();
        auto right = parseAddSub(tok);
        double lv = std::holds_alternative<double>(left) ? std::get<double>(left) : 0;
        double rv = std::holds_alternative<double>(right) ? std::get<double>(right) : 0;
        bool result = false;
        switch (t.type) {
            case TokenType::EqualsEquals: result = (lv == rv); break;
            case TokenType::NotEquals: result = (lv != rv); break;
            case TokenType::Less: result = (lv < rv); break;
            case TokenType::LessEqual: result = (lv <= rv); break;
            case TokenType::Greater: result = (lv > rv); break;
            case TokenType::GreaterEqual: result = (lv >= rv); break;
            default: break;
        }
        left = ScriptValue(result);
    }
    return left;
}

ScriptValue ExtendScriptEngine::parseAddSub(Tokenizer& tok) {
    auto left = parseMulDiv(tok);
    while (tok.peek().type == TokenType::Plus || tok.peek().type == TokenType::Minus) {
        Token op = tok.next();
        auto right = parseMulDiv(tok);
        if (op.type == TokenType::Plus) {
            if (std::holds_alternative<std::string>(left) || std::holds_alternative<std::string>(right)) {
                std::string ls = std::holds_alternative<std::string>(left) ? std::get<std::string>(left) : "";
                std::string rs = std::holds_alternative<std::string>(right) ? std::get<std::string>(right) : "";
                left = ScriptValue(ls + rs);
            } else {
                double lv = std::holds_alternative<double>(left) ? std::get<double>(left) : 0;
                double rv = std::holds_alternative<double>(right) ? std::get<double>(right) : 0;
                left = ScriptValue(lv + rv);
            }
        } else {
            double lv = std::holds_alternative<double>(left) ? std::get<double>(left) : 0;
            double rv = std::holds_alternative<double>(right) ? std::get<double>(right) : 0;
            left = ScriptValue(lv - rv);
        }
    }
    return left;
}

ScriptValue ExtendScriptEngine::parseMulDiv(Tokenizer& tok) {
    auto left = parseUnary(tok);
    while (tok.peek().type == TokenType::Star || tok.peek().type == TokenType::Slash ||
           tok.peek().type == TokenType::Percent) {
        Token op = tok.next();
        auto right = parseUnary(tok);
        double lv = std::holds_alternative<double>(left) ? std::get<double>(left) : 0;
        double rv = std::holds_alternative<double>(right) ? std::get<double>(right) : 0;
        if (op.type == TokenType::Star) left = ScriptValue(lv * rv);
        else if (op.type == TokenType::Slash) left = ScriptValue(rv != 0 ? lv / rv : 0.0);
        else left = ScriptValue(rv != 0 ? std::fmod(lv, rv) : 0.0);
    }
    return left;
}

ScriptValue ExtendScriptEngine::parseUnary(Tokenizer& tok) {
    Token t = tok.peek();
    if (t.type == TokenType::Minus) {
        tok.next();
        auto val = parsePrimary(tok);
        double v = std::holds_alternative<double>(val) ? std::get<double>(val) : 0;
        return ScriptValue(-v);
    }
    if (t.type == TokenType::Not) {
        tok.next();
        auto val = parsePrimary(tok);
        bool v = std::holds_alternative<bool>(val) ? std::get<bool>(val) :
                 (std::holds_alternative<double>(val) ? std::get<double>(val) != 0 : !std::get<std::string>(val).empty());
        return ScriptValue(!v);
    }
    return parsePrimary(tok);
}

ScriptValue ExtendScriptEngine::parseArrayLiteral(Tokenizer& tok) {
    tok.next(); // consume '['
    std::vector<double> arr;
    if (tok.peek().type != TokenType::RightBracket) {
        do {
            auto val = parseExpression(tok);
            double v = std::holds_alternative<double>(val) ? std::get<double>(val) : 0;
            arr.push_back(v);
            if (tok.peek().type == TokenType::Comma) tok.next();
        } while (tok.peek().type != TokenType::RightBracket && tok.peek().type != TokenType::Eof);
    }
    if (tok.peek().type == TokenType::RightBracket) tok.next();
    return ScriptValue(arr);
}

ScriptValue ExtendScriptEngine::parseFunctionCall(const std::string& name, Tokenizer& tok) {
    tok.next(); // consume '('
    std::vector<ScriptValue> args;
    if (tok.peek().type != TokenType::RightParen) {
        do {
            args.push_back(parseExpression(tok));
            if (tok.peek().type == TokenType::Comma) tok.next();
        } while (tok.peek().type != TokenType::RightParen && tok.peek().type != TokenType::Eof);
    }
    if (tok.peek().type == TokenType::RightParen) tok.next();

    // Check registered functions
    auto it = m_functions.find(name);
    if (it != m_functions.end()) {
        return it->second(args);
    }

    // Built-in functions
    if (name == "Math.sin" && args.size() >= 1) {
        double v = std::holds_alternative<double>(args[0]) ? std::get<double>(args[0]) : 0;
        return ScriptValue(std::sin(v));
    }
    if (name == "Math.cos" && args.size() >= 1) {
        double v = std::holds_alternative<double>(args[0]) ? std::get<double>(args[0]) : 0;
        return ScriptValue(std::cos(v));
    }
    if (name == "Math.sqrt" && args.size() >= 1) {
        double v = std::holds_alternative<double>(args[0]) ? std::get<double>(args[0]) : 0;
        return ScriptValue(std::sqrt(v));
    }
    if (name == "Math.abs" && args.size() >= 1) {
        double v = std::holds_alternative<double>(args[0]) ? std::get<double>(args[0]) : 0;
        return ScriptValue(std::abs(v));
    }
    if (name == "Math.pow" && args.size() >= 2) {
        double base = std::holds_alternative<double>(args[0]) ? std::get<double>(args[0]) : 0;
        double exp = std::holds_alternative<double>(args[1]) ? std::get<double>(args[1]) : 0;
        return ScriptValue(std::pow(base, exp));
    }
    if (name == "Math.floor" && args.size() >= 1) {
        double v = std::holds_alternative<double>(args[0]) ? std::get<double>(args[0]) : 0;
        return ScriptValue(std::floor(v));
    }
    if (name == "Math.ceil" && args.size() >= 1) {
        double v = std::holds_alternative<double>(args[0]) ? std::get<double>(args[0]) : 0;
        return ScriptValue(std::ceil(v));
    }
    if (name == "Math.round" && args.size() >= 1) {
        double v = std::holds_alternative<double>(args[0]) ? std::get<double>(args[0]) : 0;
        return ScriptValue(std::round(v));
    }
    if (name == "Math.min" && args.size() >= 2) {
        double a = std::holds_alternative<double>(args[0]) ? std::get<double>(args[0]) : 0;
        double b = std::holds_alternative<double>(args[1]) ? std::get<double>(args[1]) : 0;
        return ScriptValue(std::min(a, b));
    }
    if (name == "Math.max" && args.size() >= 2) {
        double a = std::holds_alternative<double>(args[0]) ? std::get<double>(args[0]) : 0;
        double b = std::holds_alternative<double>(args[1]) ? std::get<double>(args[1]) : 0;
        return ScriptValue(std::max(a, b));
    }
    if (name == "Math.PI") {
        return ScriptValue(M_PI);
    }
    if (name == "length" && args.size() >= 1) {
        if (std::holds_alternative<std::vector<double>>(args[0])) {
            return ScriptValue(static_cast<double>(std::get<std::vector<double>>(args[0]).size()));
        }
        if (std::holds_alternative<std::string>(args[0])) {
            return ScriptValue(static_cast<double>(std::get<std::string>(args[0]).size()));
        }
    }
    if (name == "log" && args.size() >= 1) {
        if (std::holds_alternative<std::string>(args[0])) {
            log(std::get<std::string>(args[0]));
        } else if (std::holds_alternative<double>(args[0])) {
            log(std::to_string(std::get<double>(args[0])));
        }
        return ScriptValue(0.0);
    }

    // Layer/comp/project API calls routed through registered functions
    if (name == "comp.numLayers") return ScriptValue(static_cast<double>(comp.numLayers()));
    if (name == "comp.duration") return ScriptValue(comp.duration());
    if (name == "comp.frameRate") return ScriptValue(comp.frameRate());
    if (name == "comp.width") return ScriptValue(static_cast<double>(comp.width()));
    if (name == "comp.height") return ScriptValue(static_cast<double>(comp.height()));
    if (name == "project.numItems") return ScriptValue(static_cast<double>(project.numItems()));

    if (name == "layer.name" && args.size() >= 1) {
        return ScriptValue(layer.name());
    }
    if (name == "layer.opacity") return ScriptValue(layer.opacity());
    if (name == "layer.rotation") return ScriptValue(layer.rotation());
    if (name == "layer.position") return ScriptValue(layer.position());
    if (name == "layer.scale") return ScriptValue(layer.scale());

    m_lastError = "Unknown function: " + name;
    return ScriptValue(0.0);
}

ScriptValue ExtendScriptEngine::parsePrimary(Tokenizer& tok) {
    Token t = tok.next();
    if (t.type == TokenType::Number) {
        return ScriptValue(t.numValue);
    }
    if (t.type == TokenType::String) {
        return ScriptValue(t.value);
    }
    if (t.type == TokenType::LeftParen) {
        auto val = parseExpression(tok);
        if (tok.peek().type == TokenType::RightParen) tok.next();
        return val;
    }
    if (t.type == TokenType::LeftBracket) {
        // Put the token back by re-tokenizing is expensive, so parse array inline
        std::vector<double> arr;
        if (tok.peek().type != TokenType::RightBracket) {
            do {
                auto val = parseExpression(tok);
                double v = std::holds_alternative<double>(val) ? std::get<double>(val) : 0;
                arr.push_back(v);
                if (tok.peek().type == TokenType::Comma) tok.next();
            } while (tok.peek().type != TokenType::RightBracket && tok.peek().type != TokenType::Eof);
        }
        if (tok.peek().type == TokenType::RightBracket) tok.next();
        return ScriptValue(arr);
    }
    if (t.type == TokenType::Identifier) {
        // Check if function call
        if (tok.peek().type == TokenType::LeftParen) {
            return parseFunctionCall(t.value, tok);
        }
        // Variable lookup
        auto it = m_variables.find(t.value);
        if (it != m_variables.end()) {
            return it->second;
        }
        // Dot-separated member access
        if (tok.peek().type == TokenType::Dot) {
            std::string fullName = t.value;
            while (tok.peek().type == TokenType::Dot) {
                tok.next(); // consume '.'
                Token member = tok.next();
                fullName += "." + member.value;
                if (tok.peek().type == TokenType::LeftParen) {
                    return parseFunctionCall(fullName, tok);
                }
            }
            auto fi = m_variables.find(fullName);
            if (fi != m_variables.end()) return fi->second;
            return ScriptValue(0.0);
        }
        return ScriptValue(0.0);
    }
    return ScriptValue(0.0);
}

// ─── Execute ──────────────────────────────────────────────────────────

bool ExtendScriptEngine::execute(const std::string& script) {
    std::lock_guard<std::mutex> lock(m_mutex2);
    m_lastError.clear();
    try {
        Tokenizer tok(script);
        while (tok.peek().type != TokenType::Eof) {
            Token first = tok.peek();
            // Variable assignment: identifier = expr
            if (first.type == TokenType::Identifier && tok.peek().type == TokenType::Identifier) {
                Token name = tok.next();
                Token eq = tok.peek();
                if (eq.type == TokenType::Equals) {
                    tok.next(); // consume '='
                    auto val = parseExpression(tok);
                    m_variables[name.value] = val;
                    if (tok.peek().type == TokenType::Semicolon) tok.next();
                    continue;
                }
            }
            // Standalone expression
            auto val = parseExpression(tok);
            if (tok.peek().type == TokenType::Semicolon) tok.next();
            (void)val;
        }
        return true;
    } catch (const std::exception& e) {
        m_lastError = e.what();
        return false;
    }
}

bool ExtendScriptEngine::executeFile(const std::string& filePath) {
    std::ifstream ifs(filePath);
    if (!ifs) {
        m_lastError = "Cannot open file: " + filePath;
        return false;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    return execute(ss.str());
}

void ExtendScriptEngine::setVariable(const std::string& name, const ScriptValue& value) {
    std::lock_guard<std::mutex> lock(m_mutex2);
    m_variables[name] = value;
}

ScriptValue ExtendScriptEngine::getVariable(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_mutex2);
    auto it = m_variables.find(name);
    if (it != m_variables.end()) return it->second;
    return ScriptValue(0.0);
}

void ExtendScriptEngine::registerFunction(const std::string& name,
    std::function<ScriptValue(const std::vector<ScriptValue>&)> func) {
    std::lock_guard<std::mutex> lock(m_mutex2);
    m_functions[name] = std::move(func);
}

void ExtendScriptEngine::setLogCallback(std::function<void(const std::string&)> callback) {
    m_logCallback = std::move(callback);
}

void ExtendScriptEngine::log(const std::string& message) {
    if (m_logCallback) m_logCallback(message);
}

void ExtendScriptEngine::clearOutput() {
    log("CLEAR");
}

bool ExtendScriptEngine::hasError() const {
    return !m_lastError.empty();
}

std::string ExtendScriptEngine::getLastError() const {
    return m_lastError;
}

ScriptValue ExtendScriptEngine::evalExpression(const std::string& expr) {
    try {
        Tokenizer tok(expr);
        return parseExpression(tok);
    } catch (...) {
        return ScriptValue(0.0);
    }
}

// ─── API Stubs (interact with project state when available) ───────────

int ExtendScriptEngine::ProjectAPI::numItems() const { return 0; }
std::string ExtendScriptEngine::ProjectAPI::item(int) const { return ""; }
std::string ExtendScriptEngine::ProjectAPI::getActiveItem() const { return ""; }
int ExtendScriptEngine::ProjectAPI::activeCompIndex() const { return 0; }
void ExtendScriptEngine::ProjectAPI::openProject(const std::string&) {}
void ExtendScriptEngine::ProjectAPI::saveProject(const std::string&) {}
void ExtendScriptEngine::ProjectAPI::closeProject() {}

int ExtendScriptEngine::CompAPI::numLayers() const { return 0; }
std::string ExtendScriptEngine::CompAPI::layer(int) const { return ""; }
double ExtendScriptEngine::CompAPI::duration() const { return 0; }
double ExtendScriptEngine::CompAPI::frameRate() const { return 30; }
int ExtendScriptEngine::CompAPI::width() const { return 1920; }
int ExtendScriptEngine::CompAPI::height() const { return 1080; }
void ExtendScriptEngine::CompAPI::addLayer(const std::string&, double) {}

std::string ExtendScriptEngine::LayerAPI::name() const { return ""; }
void ExtendScriptEngine::LayerAPI::setName(const std::string&) {}
double ExtendScriptEngine::LayerAPI::inPoint() const { return 0; }
double ExtendScriptEngine::LayerAPI::outPoint() const { return 0; }
void ExtendScriptEngine::LayerAPI::setInPoint(double) {}
void ExtendScriptEngine::LayerAPI::setOutPoint(double) {}
double ExtendScriptEngine::LayerAPI::opacity() const { return 100; }
void ExtendScriptEngine::LayerAPI::setOpacity(double) {}
std::vector<double> ExtendScriptEngine::LayerAPI::position() const { return {0, 0}; }
void ExtendScriptEngine::LayerAPI::setPosition(const std::vector<double>&) {}
std::vector<double> ExtendScriptEngine::LayerAPI::scale() const { return {100, 100}; }
void ExtendScriptEngine::LayerAPI::setScale(const std::vector<double>&) {}
double ExtendScriptEngine::LayerAPI::rotation() const { return 0; }
void ExtendScriptEngine::LayerAPI::setRotation(double) {}
int ExtendScriptEngine::LayerAPI::numEffects() const { return 0; }
void ExtendScriptEngine::LayerAPI::addEffect(const std::string&) {}

int ExtendScriptEngine::RenderQueueAPI::numItems() const { return 0; }
void ExtendScriptEngine::RenderQueueAPI::addItem(const std::string&) {}
void ExtendScriptEngine::RenderQueueAPI::setOutputModule(int, const std::string&) {}
void ExtendScriptEngine::RenderQueueAPI::setOutputPath(int, const std::string&) {}
void ExtendScriptEngine::RenderQueueAPI::startRendering() {}
void ExtendScriptEngine::RenderQueueAPI::stopRendering() {}

} // namespace FreeEffect
