#include "ai_connector.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QSettings>
#include <QSslSocket>

namespace FreeEffect {

static const QString SYSTEM_PROMPT = 
    "You are FreeEffect AI Assistant, an expert helper for FreeEffect — an open-source motion graphics "
    "and visual effects application similar to Adobe After Effects. You help users with:\n"
    "- Creating and managing compositions, layers, and keyframes\n"
    "- Explaining keyboard shortcuts and workflows\n"
    "- Writing expressions for layer properties\n"
    "- Troubleshooting rendering and import issues\n"
    "- Suggesting best practices for motion graphics\n"
    "- General VFX and compositing knowledge\n\n"
    "Be concise, helpful, and format your responses with markdown when appropriate. "
    "When suggesting keyboard shortcuts, use the format: (Key). "
    "When suggesting menu actions, use the format: Menu > Action.";

AIConnector::AIConnector(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_systemPrompt(SYSTEM_PROMPT) {
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &AIConnector::onReplyFinished);
    
    // Load saved config
    QSettings settings("FreeEffect", "FreeEffect");
    m_config.provider = static_cast<AIProvider>(settings.value("ai/provider", 2).toInt());
    m_config.apiKey = settings.value("ai/apiKey").toString();
    m_config.model = settings.value("ai/model").toString();
    m_config.baseUrl = settings.value("ai/baseUrl").toString();
    m_config.maxTokens = settings.value("ai/maxTokens", 2048).toInt();
    m_config.temperature = settings.value("ai/temperature", 0.7).toDouble();
    
    if (m_config.model.isEmpty()) m_config = AIConfig::defaults(m_config.provider);
}

AIConnector::~AIConnector() {
    cancelCurrentRequest();
}

void AIConnector::setConfig(const AIConfig& config) {
    m_config = config;
    
    QSettings settings("FreeEffect", "FreeEffect");
    settings.setValue("ai/provider", static_cast<int>(m_config.provider));
    settings.setValue("ai/apiKey", m_config.apiKey);
    settings.setValue("ai/model", m_config.model);
    settings.setValue("ai/baseUrl", m_config.baseUrl);
    settings.setValue("ai/maxTokens", m_config.maxTokens);
    settings.setValue("ai/temperature", m_config.temperature);
}

void AIConnector::cancelCurrentRequest() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    emit requestFinished();
}

void AIConnector::sendMessage(const QString& userMessage) {
    if (userMessage.trimmed().isEmpty()) return;
    
    m_history.append({"user", userMessage});
    emit requestStarted();
    emit requestFinished();
    
    switch (m_config.provider) {
        case AIProvider::OpenAI: sendOpenAIRequest(userMessage); break;
        case AIProvider::Anthropic: sendAnthropicRequest(userMessage); break;
        case AIProvider::OllamaLocal: sendOllamaRequest(userMessage); break;
    }
}

void AIConnector::sendOpenAIRequest(const QString& message) {
    QJsonObject body;
    body["model"] = m_config.model;
    body["max_tokens"] = m_config.maxTokens;
    body["temperature"] = m_config.temperature;
    
    QJsonArray messages;
    messages.append(QJsonObject{{"role", "system"}, {"content", m_systemPrompt}});
    for (const auto& msg : m_history) {
        messages.append(QJsonObject{{"role", msg.role}, {"content", msg.content}});
    }
    body["messages"] = messages;
    
    QNetworkRequest request(QUrl(m_config.baseUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_config.apiKey).toUtf8());
    
    m_currentReply = m_networkManager->post(request, QJsonDocument(body).toJson());
}

void AIConnector::sendAnthropicRequest(const QString& message) {
    QJsonObject body;
    body["model"] = m_config.model;
    body["max_tokens"] = m_config.maxTokens;
    body["temperature"] = m_config.temperature;
    body["system"] = m_systemPrompt;
    
    QJsonArray messages;
    for (const auto& msg : m_history) {
        messages.append(QJsonObject{{"role", msg.role}, {"content", msg.content}});
    }
    body["messages"] = messages;
    
    QNetworkRequest request(QUrl(m_config.baseUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-api-key", m_config.apiKey.toUtf8());
    request.setRawHeader("anthropic-version", "2023-06-01");
    
    m_currentReply = m_networkManager->post(request, QJsonDocument(body).toJson());
}

void AIConnector::sendOllamaRequest(const QString& message) {
    QJsonObject body;
    body["model"] = m_config.model;
    body["stream"] = false;
    
    QJsonArray messages;
    messages.append(QJsonObject{{"role", "system"}, {"content", m_systemPrompt}});
    for (const auto& msg : m_history) {
        messages.append(QJsonObject{{"role", msg.role}, {"content", msg.content}});
    }
    body["messages"] = messages;
    
    QNetworkRequest request(QUrl(m_config.baseUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    m_currentReply = m_networkManager->post(request, QJsonDocument(body).toJson());
}

void AIConnector::onReplyFinished(QNetworkReply* reply) {
    if (reply != m_currentReply) return;
    
    QByteArray data = reply->readAll();
    reply->deleteLater();
    m_currentReply = nullptr;
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg;
        if (reply->error() == QNetworkReply::ConnectionRefusedError) {
            errorMsg = "Connection refused. Make sure your AI service is running.\n"
                       "For Ollama: run 'ollama serve' in terminal.";
        } else {
            errorMsg = "Error: " + reply->errorString() + "\n" + QString::fromUtf8(data);
        }
        emit errorOccurred(errorMsg);
        emit requestFinished();
        return;
    }
    
    switch (m_config.provider) {
        case AIProvider::OpenAI: processOpenAIResponse(data); break;
        case AIProvider::Anthropic: processAnthropicResponse(data); break;
        case AIProvider::OllamaLocal: processOllamaResponse(data); break;
    }
    
    emit requestFinished();
}

void AIConnector::processOpenAIResponse(const QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    if (obj.contains("error")) {
        emit errorOccurred("API Error: " + obj["error"].toObject()["message"].toString());
        return;
    }
    
    QString content = obj["choices"].toArray()[0].toObject()["message"].toObject()["content"].toString();
    m_history.append({"assistant", content});
    emit responseReceived(content);
}

void AIConnector::processAnthropicResponse(const QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    if (obj.contains("error")) {
        emit errorOccurred("API Error: " + obj["error"].toObject()["message"].toString());
        return;
    }
    
    QString content;
    for (const auto& block : obj["content"].toArray()) {
        if (block.toObject()["type"].toString() == "text") {
            content += block.toObject()["text"].toString();
        }
    }
    m_history.append({"assistant", content});
    emit responseReceived(content);
}

void AIConnector::processOllamaResponse(const QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    
    if (obj.contains("error")) {
        emit errorOccurred("Ollama Error: " + obj["error"].toString());
        return;
    }
    
    QString content = obj["message"].toObject()["content"].toString();
    m_history.append({"assistant", content});
    emit responseReceived(content);
}

void AIConnector::onReadyRead() {
    // Streaming support (future)
}

} // namespace FreeEffect
