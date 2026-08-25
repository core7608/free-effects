#pragma once

#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QApplication>

namespace FreeEffect {

class SplashScreen : public QSplashScreen {
    Q_OBJECT
public:
    explicit SplashScreen(const QPixmap& pixmap = QPixmap());
    
    void showMessage(const QString& message, int timeout = 2000);
    void finishWithDelay();

private slots:
    void onTimeout();

private:
    void setupOverlay();
    
    QLabel* m_statusLabel = nullptr;
    QProgressBar* m_progressBar = nullptr;
    int m_progress = 0;
    QTimer m_timer;
};

} // namespace FreeEffect
