#include "splash_screen.h"
#include <QPainter>

namespace FreeEffect {

SplashScreen::SplashScreen(const QPixmap& pixmap)
    : QSplashScreen(pixmap) {
    setupOverlay();
    setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

void SplashScreen::setupOverlay() {
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: #555555; font-size: 11px; background: transparent;");
    m_statusLabel->setGeometry(60, pixmap().height() - 36, pixmap().width() - 120, 16);
    m_statusLabel->setText("Initializing...");

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setFixedHeight(2);
    m_progressBar->setStyleSheet(
        "QProgressBar { background: transparent; border: none; }"
        "QProgressBar::chunk { background: #ffffff; border-radius: 1px; }"
    );
    m_progressBar->setTextVisible(false);
    m_progressBar->setGeometry(60, pixmap().height() - 14, pixmap().width() - 120, 2);

    connect(&m_timer, &QTimer::timeout, this, &SplashScreen::onTimeout);
    m_timer.start(30);
}

void SplashScreen::showMessage(const QString& message, int timeout) {
    Q_UNUSED(timeout);
    if (m_statusLabel) {
        m_statusLabel->setText(message);
        repaint();
        QApplication::processEvents();
    }
}

void SplashScreen::onTimeout() {
    m_progress = std::min(m_progress + 2, 100);
    if (m_progressBar) {
        m_progressBar->setValue(m_progress);
        repaint();
    }
}

void SplashScreen::finishWithDelay() {
    m_timer.stop();
    if (m_progressBar) m_progressBar->setValue(100);
    QTimer::singleShot(500, this, [this]() { close(); });
}

} // namespace FreeEffect
