#include "ai_panel.h"
#include "ai_command_executor.h"
#include "../mainwindow/main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QScrollBar>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace FreeEffect {

AIPanel::AIPanel(MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent)
    , m_connector(std::make_unique<AIConnector>(this))
    , m_commandExecutor(std::make_unique<AICommandExecutor>(parent)) {
    
    setupUi();
    
    connect(m_connector.get(), &AIConnector::responseReceived, this, &AIPanel::onResponseReceived);
    connect(m_connector.get(), &AIConnector::errorOccurred, this, &AIPanel::onErrorOccurred);
    connect(m_connector.get(), &AIConnector::requestStarted, this, [this]() {
        m_sendBtn->setEnabled(false);
        m_sendBtn->setText("...");
        m_receivingResponse = true;
    });
    connect(m_connector.get(), &AIConnector::requestFinished, this, [this]() {
        m_sendBtn->setEnabled(true);
        m_sendBtn->setText("Send");
        m_receivingResponse = false;
    });
    
    connect(m_commandExecutor.get(), &AICommandExecutor::message, this, [this](const QString& msg) {
        addSystemMessage(msg);
    });
    connect(m_commandExecutor.get(), &AICommandExecutor::commandExecuted, this, [this](const QString& action, bool success) {
        if (success) {
            addSystemMessage("Command executed: " + action);
        } else {
            addSystemMessage("Command failed: " + action);
        }
    });
    
    // Initial greeting
    addSystemMessage("FreeEffect AI Assistant ready. Ask me anything about motion graphics, VFX, or how to use FreeEffect.");
}

void AIPanel::setupUi() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    setupHeader();
    setupChatArea();
    setupInputArea();
}

void AIPanel::setupHeader() {
    QWidget* header = new QWidget(this);
    header->setFixedHeight(36);
    header->setStyleSheet("background-color: #111111; border-bottom: 1px solid #1a1a1a;");
    
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(8, 0, 8, 0);
    headerLayout->setSpacing(6);
    
    QLabel* aiIcon = new QLabel("AI", header);
    aiIcon->setStyleSheet(
        "background-color: #ffffff; color: #000000; font-weight: 900; font-size: 9px; "
        "padding: 2px 4px; border-radius: 2px;"
    );
    aiIcon->setFixedWidth(22);
    aiIcon->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(aiIcon);
    
    m_providerLabel = new QLabel("Assistant", header);
    m_providerLabel->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: 600; background: transparent;");
    headerLayout->addWidget(m_providerLabel);
    
    m_providerCombo = new QComboBox(header);
    m_providerCombo->addItems({"Ollama (Local)", "OpenAI", "Anthropic"});
    m_providerCombo->setFixedWidth(130);
    m_providerCombo->setStyleSheet(
        "QComboBox { background: #1a1a1a; color: #888888; border: 1px solid #2a2a2a; "
        "border-radius: 2px; padding: 2px 6px; font-size: 10px; }"
        "QComboBox:hover { border-color: #444444; }"
    );
    connect(m_providerCombo, QOverload<int>::of(&QComboBox::activated), this, &AIPanel::onProviderChanged);
    headerLayout->addWidget(m_providerCombo);
    
    m_clearBtn = new QPushButton("Clear", header);
    m_clearBtn->setFixedWidth(44);
    m_clearBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #555555; border: none; font-size: 10px; }"
        "QPushButton:hover { color: #ffffff; }"
    );
    connect(m_clearBtn, &QPushButton::clicked, this, &AIPanel::onClearChat);
    headerLayout->addWidget(m_clearBtn);
    
    m_mainLayout->addWidget(header);
}

void AIPanel::setupChatArea() {
    m_chatBrowser = new QTextBrowser(this);
    m_chatBrowser->setOpenExternalLinks(true);
    m_chatBrowser->setStyleSheet(
        "QTextBrowser { background-color: #0e0e0e; color: #cccccc; border: none; "
        "padding: 12px; font-size: 12px; selection-background-color: #ffffff; selection-color: #000000; }"
    );
    m_chatBrowser->document()->setDefaultStyleSheet(
        "body { color: #cccccc; font-family: 'SF Pro Display', 'Segoe UI', Arial, sans-serif; }"
        "p { margin: 4px 0; line-height: 1.4; }"
        "code { background: #1a1a1a; color: #e0e0e0; padding: 1px 4px; border-radius: 2px; font-size: 11px; }"
        "pre { background: #1a1a1a; color: #e0e0e0; padding: 8px; border-radius: 2px; "
        "font-family: 'SF Mono', 'Menlo', monospace; font-size: 11px; }"
        "b { color: #ffffff; }"
        "a { color: #ffffff; }"
    );
    m_mainLayout->addWidget(m_chatBrowser, 1);
}

void AIPanel::setupInputArea() {
    QWidget* inputWidget = new QWidget(this);
    inputWidget->setStyleSheet("background-color: #0e0e0e; border-top: 1px solid #1a1a1a;");
    
    QVBoxLayout* inputLayout = new QVBoxLayout(inputWidget);
    inputLayout->setContentsMargins(8, 8, 8, 8);
    inputLayout->setSpacing(6);
    
    m_inputEdit = new QLineEdit(inputWidget);
    m_inputEdit->setPlaceholderText("Ask about FreeEffect, VFX, expressions...");
    m_inputEdit->setStyleSheet(
        "QLineEdit { background-color: #141414; color: #e0e0e0; border: 1px solid #2a2a2a; "
        "border-radius: 2px; padding: 8px 12px; font-size: 12px; }"
        "QLineEdit:focus { border: 1px solid #ffffff; }"
    );
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &AIPanel::onInputReturnPressed);
    inputLayout->addWidget(m_inputEdit);
    
    QWidget* btnRow = new QWidget(inputWidget);
    btnRow->setFixedHeight(26);
    QHBoxLayout* btnLayout = new QHBoxLayout(btnRow);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(4);
    
    m_statusLabel = new QLabel("Ready", btnRow);
    m_statusLabel->setStyleSheet("color: #444444; font-size: 10px; background: transparent;");
    btnLayout->addWidget(m_statusLabel);
    btnLayout->addStretch();
    
    m_sendBtn = new QPushButton("Send", btnRow);
    m_sendBtn->setFixedSize(60, 24);
    m_sendBtn->setStyleSheet(
        "QPushButton { background-color: #ffffff; color: #000000; border: none; "
        "border-radius: 2px; font-size: 11px; font-weight: bold; }"
        "QPushButton:hover { background-color: #e0e0e0; }"
        "QPushButton:disabled { background-color: #333333; color: #555555; }"
    );
    connect(m_sendBtn, &QPushButton::clicked, this, &AIPanel::onSendClicked);
    btnLayout->addWidget(m_sendBtn);
    
    inputLayout->addWidget(btnRow);
    m_mainLayout->addWidget(inputWidget);
}

void AIPanel::onSendClicked() {
    onInputReturnPressed();
}

void AIPanel::onInputReturnPressed() {
    QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty() || m_receivingResponse) return;
    
    m_inputEdit->clear();
    sendMessage(text);
}

void AIPanel::sendMessage(const QString& text) {
    addMessage("user", text);
    
    // Check if this is a local command
    if (m_commandExecutor && m_commandExecutor->parseAndExecute(text)) {
        return; // Command was executed locally, no need to call AI
    }
    
    m_statusLabel->setText("Thinking...");
    m_connector->sendMessage(text);
}

void AIPanel::onResponseReceived(const QString& response) {
    addMessage("assistant", response);
    m_statusLabel->setText("Ready");
}

void AIPanel::onResponseChunk(const QString& chunk) {
    Q_UNUSED(chunk);
}

void AIPanel::onErrorOccurred(const QString& error) {
    addSystemMessage("Error: " + error);
    m_statusLabel->setText("Error");
}

void AIPanel::onProviderChanged(int index) {
    AIProvider providers[] = {AIProvider::OllamaLocal, AIProvider::OpenAI, AIProvider::Anthropic};
    AIConfig config = AIConfig::defaults(providers[index]);
    
    QSettings settings("FreeEffect", "FreeEffect");
    config.apiKey = settings.value("ai/apiKey").toString();
    
    m_connector->setConfig(config);
    addSystemMessage("Switched to " + m_providerCombo->currentText());
}

void AIPanel::onClearChat() {
    m_chatBrowser->clear();
    m_connector->clearHistory();
    addSystemMessage("Chat cleared. How can I help you?");
}

void AIPanel::onSettingsToggled() {}

void AIPanel::addMessage(const QString& role, const QString& content) {
    QString html;
    if (role == "user") {
        html = QString(
            "<div style='margin: 8px 0; padding: 8px 12px; background: #1a1a1a; border-radius: 4px; "
            "border-left: 3px solid #ffffff;'>"
            "<b style='color: #ffffff; font-size: 10px;'>You</b><br>"
            "<span style='color: #cccccc;'>%1</span>"
            "</div>"
        ).arg(content.toHtmlEscaped());
    } else {
        html = QString(
            "<div style='margin: 8px 0; padding: 8px 12px; background: #111111; border-radius: 4px; "
            "border-left: 3px solid #444444;'>"
            "<b style='color: #888888; font-size: 10px;'>AI Assistant</b><br>"
            "<span style='color: #cccccc;'>%1</span>"
            "</div>"
        ).arg(formatResponse(content));
    }
    
    m_chatBrowser->append(html);
    QScrollBar* sb = m_chatBrowser->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void AIPanel::addSystemMessage(const QString& text) {
    QString html = QString(
        "<div style='margin: 4px 0; padding: 4px 8px; text-align: center;'>"
        "<span style='color: #444444; font-size: 10px;'>%1</span>"
        "</div>"
    ).arg(text.toHtmlEscaped());
    m_chatBrowser->append(html);
}

QString AIPanel::formatResponse(const QString& text) {
    QString formatted = text.toHtmlEscaped();
    
    // Bold **text**
    QRegularExpression boldRegex("\\*\\*(.*?)\\*\\*");
    formatted.replace(boldRegex, "<b>\\1</b>");
    
    // Inline code `code`
    QRegularExpression codeRegex("`([^`]+)`");
    formatted.replace(codeRegex, "<code>\\1</code>");
    
    // Code blocks ```code```
    QRegularExpression codeBlockRegex("```(\\w*)\\n(.*?)```", QRegularExpression::DotMatchesEverythingOption);
    formatted.replace(codeBlockRegex, "<pre>\\2</pre>");
    
    // Line breaks
    formatted.replace("\n", "<br>");
    
    return formatted;
}

} // namespace FreeEffect
