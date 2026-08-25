#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <memory>
#include "ai_connector.h"

namespace FreeEffect {

class MainWindow;
class AICommandExecutor;

class AIPanel : public QWidget {
    Q_OBJECT
public:
    explicit AIPanel(MainWindow* parent);
    ~AIPanel() override = default;
    
    AIConnector* getConnector() { return m_connector.get(); }
    AICommandExecutor* getCommandExecutor() { return m_commandExecutor.get(); }
    void sendMessage(const QString& text);
    
    static QString getSystemPrompts(int index);

signals:
    void commandRequested(const QString& command);

private slots:
    void onSendClicked();
    void onInputReturnPressed();
    void onResponseReceived(const QString& response);
    void onResponseChunk(const QString& chunk);
    void onErrorOccurred(const QString& error);
    void onProviderChanged(int index);
    void onClearChat();
    void onSettingsToggled();

private:
    void setupUi();
    void setupHeader();
    void setupChatArea();
    void setupInputArea();
    void addMessage(const QString& role, const QString& content);
    void addSystemMessage(const QString& text);
    QString formatResponse(const QString& text);
    
    MainWindow* m_mainWindow;
    std::unique_ptr<AIConnector> m_connector;
    std::unique_ptr<AICommandExecutor> m_commandExecutor;
    QVBoxLayout* m_mainLayout = nullptr;
    
    QLabel* m_statusLabel;
    QLabel* m_providerLabel;
    QComboBox* m_providerCombo;
    QTextBrowser* m_chatBrowser;
    QLineEdit* m_inputEdit;
    QPushButton* m_sendBtn;
    QPushButton* m_clearBtn;
    QString m_accumulatedResponse;
    bool m_receivingResponse = false;
};

} // namespace FreeEffect
