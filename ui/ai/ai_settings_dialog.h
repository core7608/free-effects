#pragma once

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include "ai_connector.h"

namespace FreeEffect {

class AISettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit AISettingsDialog(AIConnector* connector, QWidget* parent = nullptr);
    
    AIConfig getConfig() const;

private slots:
    void onProviderChanged(int index);
    void onTestConnection();
    void onAccepted();

private:
    void setupUi();
    
    AIConnector* m_connector;
    QComboBox* m_providerCombo;
    QLineEdit* m_apiKeyEdit;
    QLineEdit* m_modelEdit;
    QLineEdit* m_baseUrlEdit;
    QSpinBox* m_maxTokensSpin;
    QDoubleSpinBox* m_temperatureSpin;
    QLabel* m_statusLabel;
    QPushButton* m_testBtn;
};

} // namespace FreeEffect
