#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

namespace FreeEffect {

enum class AIProvider {
    OpenAI,
    Anthropic,
    OllamaLocal
};

struct AIConfig {
    AIProvider provider = AIProvider::OllamaLocal;
    QString apiKey;
    QString model;
    QString baseUrl;
    int maxTokens = 2048;
    double temperature = 0.7;
    
    static AIConfig defaults(AIProvider p) {
        AIConfig c;
        c.provider = p;
        switch (p) {
            case AIProvider::OpenAI:
                c.model = "gpt-4o";
                c.baseUrl = "https://api.openai.com/v1/chat/completions";
                break;
            case AIProvider::Anthropic:
                c.model = "claude-sonnet-4-20250514";
                c.baseUrl = "https://api.anthropic.com/v1/messages";
                break;
            case AIProvider::OllamaLocal:
                c.model = "llama3.1";
                c.baseUrl = "http://localhost:11434/api/chat";
                break;
        }
        return c;
    }
};

class AIConnector : public QObject {
    Q_OBJECT
public:
    explicit AIConnector(QObject* parent = nullptr);
    ~AIConnector() override;
    
    void setConfig(const AIConfig& config);
    AIConfig getConfig() const { return m_config; }
    bool isConnected() const { return m_connected; }
    
    void sendMessage(const QString& userMessage);
    void cancelCurrentRequest();
    
    struct ChatMessage {
        QString role;
        QString content;
    };
    
    void setSystemPrompt(const QString& prompt) { m_systemPrompt = prompt; }
    void clearHistory() { m_history.clear(); }

signals:
    void responseReceived(const QString& response);
    void responseChunk(const QString& chunk);
    void errorOccurred(const QString& error);
    void connectionStatusChanged(bool connected);
    void requestStarted();
    void requestFinished();

private slots:
    void onReplyFinished(QNetworkReply* reply);
    void onReadyRead();

private:
    void sendOpenAIRequest(const QString& message);
    void sendAnthropicRequest(const QString& message);
    void sendOllamaRequest(const QString& message);
    void processOpenAIResponse(const QByteArray& data);
    void processAnthropicResponse(const QByteArray& data);
    void processOllamaResponse(const QByteArray& data);
    
    AIConfig m_config;
    QNetworkAccessManager* m_networkManager = nullptr;
    QNetworkReply* m_currentReply = nullptr;
    QList<ChatMessage> m_history;
    QString m_systemPrompt;
    QString m_bufferedResponse;
    bool m_connected = false;
    bool m_streaming = false;
};

} // namespace FreeEffect
