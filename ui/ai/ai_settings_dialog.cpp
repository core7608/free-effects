#include "ai_settings_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

namespace FreeEffect {

AISettingsDialog::AISettingsDialog(AIConnector* connector, QWidget* parent)
    : QDialog(parent)
    , m_connector(connector) {
    setWindowTitle("AI Assistant Settings");
    setMinimumWidth(500);
    setStyleSheet("background-color: #0e0e0e; color: #cccccc;");
    setupUi();
}

void AISettingsDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Provider
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(8);
    
    grid->addWidget(new QLabel("Provider:", this), 0, 0);
    m_providerCombo = new QComboBox(this);
    m_providerCombo->addItems({"Ollama (Local - Free)", "OpenAI", "Anthropic"});
    connect(m_providerCombo, QOverload<int>::of(&QComboBox::activated), this, &AISettingsDialog::onProviderChanged);
    grid->addWidget(m_providerCombo, 0, 1);
    
    // API Key
    grid->addWidget(new QLabel("API Key:", this), 1, 0);
    m_apiKeyEdit = new QLineEdit(this);
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText("Not needed for Ollama");
    grid->addWidget(m_apiKeyEdit, 1, 1);
    
    // Model
    grid->addWidget(new QLabel("Model:", this), 2, 0);
    m_modelEdit = new QLineEdit(this);
    grid->addWidget(m_modelEdit, 2, 1);
    
    // Base URL
    grid->addWidget(new QLabel("Base URL:", this), 3, 0);
    m_baseUrlEdit = new QLineEdit(this);
    grid->addWidget(m_baseUrlEdit, 3, 1);
    
    // Max Tokens
    grid->addWidget(new QLabel("Max Tokens:", this), 4, 0);
    m_maxTokensSpin = new QSpinBox(this);
    m_maxTokensSpin->setRange(256, 8192);
    m_maxTokensSpin->setValue(2048);
    grid->addWidget(m_maxTokensSpin, 4, 1);
    
    // Temperature
    grid->addWidget(new QLabel("Temperature:", this), 5, 0);
    m_temperatureSpin = new QDoubleSpinBox(this);
    m_temperatureSpin->setRange(0.0, 2.0);
    m_temperatureSpin->setSingleStep(0.1);
    m_temperatureSpin->setValue(0.7);
    grid->addWidget(m_temperatureSpin, 5, 1);
    
    mainLayout->addLayout(grid);
    mainLayout->addSpacing(12);
    
    // Test connection
    QHBoxLayout* testLayout = new QHBoxLayout();
    m_testBtn = new QPushButton("Test Connection", this);
    connect(m_testBtn, &QPushButton::clicked, this, &AISettingsDialog::onTestConnection);
    testLayout->addWidget(m_testBtn);
    
    m_statusLabel = new QLabel("", this);
    testLayout->addWidget(m_statusLabel);
    testLayout->addStretch();
    mainLayout->addLayout(testLayout);
    
    mainLayout->addSpacing(12);
    
    // Info
    QLabel* infoLabel = new QLabel(
        "Setup Instructions:\n"
        "• Ollama (Local): Install Ollama from ollama.ai, then run: ollama pull llama3.1\n"
        "• OpenAI: Get API key from platform.openai.com\n"
        "• Anthropic: Get API key from console.anthropic.com", this);
    infoLabel->setStyleSheet("color: #555555; font-size: 10px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);
    
    mainLayout->addSpacing(8);
    
    // Buttons
    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->setStyleSheet(
        "QPushButton { background: #1a1a1a; color: #cccccc; border: 1px solid #2a2a2a; "
        "border-radius: 2px; padding: 6px 16px; }"
        "QPushButton:hover { background: #252525; }"
        "QPushButton:default { background: #ffffff; color: #000000; border: none; font-weight: bold; }"
    );
    connect(buttons, &QDialogButtonBox::accepted, this, &AISettingsDialog::onAccepted);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);
    
    // Load current config
    AIConfig config = m_connector->getConfig();
    m_providerCombo->setCurrentIndex(static_cast<int>(config.provider));
    m_apiKeyEdit->setText(config.apiKey);
    m_modelEdit->setText(config.model);
    m_baseUrlEdit->setText(config.baseUrl);
    m_maxTokensSpin->setValue(config.maxTokens);
    m_temperatureSpin->setValue(config.temperature);
    
    onProviderChanged(static_cast<int>(config.provider));
}

void AISettingsDialog::onProviderChanged(int index) {
    AIProvider p = static_cast<AIProvider>(index);
    AIConfig defaults = AIConfig::defaults(p);
    
    m_modelEdit->setText(defaults.model);
    m_baseUrlEdit->setText(defaults.baseUrl);
    
    bool needsKey = (p != AIProvider::OllamaLocal);
    m_apiKeyEdit->setEnabled(needsKey);
    m_apiKeyEdit->setPlaceholderText(needsKey ? "Enter API key" : "Not needed for local Ollama");
}

void AISettingsDialog::onTestConnection() {
    m_statusLabel->setText("Testing...");
    m_statusLabel->setStyleSheet("color: #888888; font-size: 11px;");
    
    AIConfig testConfig = getConfig();
    
    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::finished, this, [this, nam](QNetworkReply* reply) {
        nam->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            m_statusLabel->setText("Connected!");
            m_statusLabel->setStyleSheet("color: #ffffff; font-size: 11px; font-weight: bold;");
        } else {
            m_statusLabel->setText("Failed: " + reply->errorString().left(50));
            m_statusLabel->setStyleSheet("color: #666666; font-size: 11px;");
        }
        reply->deleteLater();
    });
    
    if (testConfig.provider == AIProvider::OllamaLocal) {
        QNetworkRequest request(QUrl("http://localhost:11434/api/tags"));
        nam->get(request);
    } else {
        QJsonObject body;
        body["model"] = testConfig.model;
        body["messages"] = QJsonArray({QJsonObject{{"role", "user"}, {"content", "Hi"}}});
        body["max_tokens"] = 5;
        
        QNetworkRequest request(QUrl(testConfig.baseUrl));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        if (testConfig.provider == AIProvider::OpenAI) {
            request.setRawHeader("Authorization", ("Bearer " + testConfig.apiKey).toUtf8());
        } else if (testConfig.provider == AIProvider::Anthropic) {
            request.setRawHeader("x-api-key", testConfig.apiKey.toUtf8());
            request.setRawHeader("anthropic-version", "2023-06-01");
        }
        
        nam->post(request, QJsonDocument(body).toJson());
    }
}

AIConfig AISettingsDialog::getConfig() const {
    AIConfig config;
    config.provider = static_cast<AIProvider>(m_providerCombo->currentIndex());
    config.apiKey = m_apiKeyEdit->text();
    config.model = m_modelEdit->text();
    config.baseUrl = m_baseUrlEdit->text();
    config.maxTokens = m_maxTokensSpin->value();
    config.temperature = m_temperatureSpin->value();
    return config;
}

void AISettingsDialog::onAccepted() {
    m_connector->setConfig(getConfig());
    accept();
}

} // namespace FreeEffect
