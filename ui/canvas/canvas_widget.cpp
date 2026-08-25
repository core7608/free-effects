#include "canvas_widget.h"
#include <QPainter>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <cmath>

namespace FreeEffect {

CanvasWidget::CanvasWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(320, 240);
    setAutoFillBackground(true);
    setAcceptDrops(true);
    
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(25, 25, 25));
    setPalette(pal);
    setMouseTracking(true);
}

void CanvasWidget::setComposition(std::shared_ptr<Composition> comp) {
    m_composition = comp;
    m_frameImage = QImage();
    m_checkerPattern = QImage();
    update();
}

void CanvasWidget::setCurrentTime(double timeInSeconds) {
    m_currentTime = timeInSeconds;
    renderFrame();
    update();
}

void CanvasWidget::setZoom(double zoom) {
    m_zoom = zoom;
    emit zoomChanged(zoom);
    update();
}

void CanvasWidget::setResolution(int quality) {
    m_renderer.setQuality(quality);
    renderFrame();
    update();
}

void CanvasWidget::setShowGrid(bool show) { m_showGrid = show; update(); }
void CanvasWidget::setShowRulers(bool show) { m_showRulers = show; update(); }

void CanvasWidget::fitToWindow() {
    if (m_composition) {
        int compW = m_composition->getResolution().width;
        int compH = m_composition->getResolution().height;
        if (compW > 0 && compH > 0) {
            double scaleX = static_cast<double>(width()) / compW;
            double scaleY = static_cast<double>(height()) / compH;
            m_zoom = std::min(scaleX, scaleY) / 0.9;
        }
    } else {
        m_zoom = 1.0;
    }
    m_offset = QPointF(0, 0);
    emit zoomChanged(m_zoom);
    update();
}

void CanvasWidget::zoomIn() {
    m_zoom = std::min(m_zoom + 0.1, 10.0);
    emit zoomChanged(m_zoom);
    update();
}

void CanvasWidget::zoomOut() {
    m_zoom = std::max(m_zoom - 0.1, 0.1);
    emit zoomChanged(m_zoom);
    update();
}

void CanvasWidget::renderFrame() {
    if (!m_composition) return;
    
    PixelBuffer buffer = m_renderer.renderFrame(*m_composition, m_currentTime);
    
    if (buffer.width > 0 && buffer.height > 0 && 
        buffer.data.size() >= static_cast<size_t>(buffer.width * buffer.height * 4)) {
        m_frameImage = QImage(buffer.width, buffer.height, QImage::Format_RGBA8888);
        memcpy(m_frameImage.bits(), buffer.data.data(), buffer.data.size());
    }
}

void CanvasWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(rect(), QColor(25, 25, 25));
    
    // Draw checkerboard where composition will be
    if (!m_composition) {
        drawCheckerboard(painter, rect());
        
        // Empty state with styled prompt
        QRect centerRect = rect().adjusted(0, -40, 0, -40);
        
        painter.setPen(QColor(60, 60, 60));
        QFont iconFont("Segoe UI", 28);
        painter.setFont(iconFont);
        painter.drawText(centerRect, Qt::AlignBottom | Qt::AlignHCenter, "+");
        
        QRect textRect = rect().adjusted(0, 20, 0, 0);
        painter.setPen(QColor(100, 100, 100));
        QFont textFont("SF Pro Display", 12);
        painter.setFont(textFont);
        painter.drawText(textRect, Qt::AlignTop | Qt::AlignHCenter, "No Composition");
        
        painter.setPen(QColor(60, 60, 60));
        QFont hintFont("SF Pro Display", 10);
        painter.setFont(hintFont);
        painter.drawText(textRect.adjusted(0, 22, 0, 0), Qt::AlignTop | Qt::AlignHCenter, 
            "Ctrl+N to create one  |  Drag & drop to import");
        
        if (m_showRulers) drawRulers(painter);
        return;
    }
    
    int compW = m_composition->getResolution().width;
    int compH = m_composition->getResolution().height;
    
    double scaleX = static_cast<double>(width()) / compW;
    double scaleY = static_cast<double>(height()) / compH;
    double scale = std::min(scaleX, scaleY) * m_zoom * 0.9;
    
    int drawW = static_cast<int>(compW * scale);
    int drawH = static_cast<int>(compH * scale);
    int drawX = (width() - drawW) / 2 + static_cast<int>(m_offset.x());
    int drawY = (height() - drawH) / 2 + static_cast<int>(m_offset.y());
    
    QRect targetRect(drawX, drawY, drawW, drawH);
    
    // Draw checkerboard behind composition
    drawCheckerboard(painter, targetRect);
    
    // Draw composition frame
    if (!m_frameImage.isNull()) {
        painter.drawImage(targetRect, m_frameImage);
    }
    
    // Draw composition border (thin dark line)
    painter.setPen(QPen(QColor(60, 60, 60), 1));
    painter.drawRect(targetRect);
    
    // Draw safe margins (AE-style dashed lines)
    QPen dashPen(QColor(100, 100, 100, 100), 1, Qt::DashLine);
    painter.setPen(dashPen);
    int margin50W = drawW / 10;
    int margin50H = drawH / 10;
    painter.drawRect(targetRect.adjusted(margin50W, margin50H, -margin50W, -margin50H));
    
    int margin90W = drawW / 20;
    int margin90H = drawH / 20;
    painter.drawRect(targetRect.adjusted(margin90W, margin90H, -margin90W, -margin90H));
    
    if (m_showGrid) drawGrid(painter, targetRect);
    if (m_showRulers) drawRulers(painter);
    
    // Time indicator overlay (bottom-left)
    if (m_composition) {
        int totalFrames = static_cast<int>(m_currentTime * m_composition->getFrameRate().fps);
        int fps = static_cast<int>(m_composition->getFrameRate().fps);
        if (fps <= 0) fps = 30;
        int frames = totalFrames % fps;
        int seconds = (totalFrames / fps) % 60;
        int minutes = (totalFrames / (fps * 60)) % 60;
        
        QString timeStr = QString("%1:%2:%3")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'))
            .arg(frames, 2, 10, QChar('0'));
        
        QRect timeRect(8, height() - 28, 70, 20);
        painter.fillRect(timeRect, QColor(0, 0, 0, 160));
        painter.setPen(QColor(200, 200, 200));
        painter.setFont(QFont("Menlo", 9));
        painter.drawText(timeRect, Qt::AlignCenter, timeStr);
    }
}

void CanvasWidget::drawCheckerboard(QPainter& painter, const QRect& rect) {
    // Cache the checkerboard pattern for performance
    if (m_checkerPattern.isNull() || m_checkerPattern.width() != 16) {
        m_checkerPattern = QImage(16, 16, QImage::Format_RGB32);
        QPainter p(&m_checkerPattern);
        p.fillRect(0, 0, 8, 8, QColor(45, 45, 45));
        p.fillRect(8, 0, 8, 8, QColor(35, 35, 35));
        p.fillRect(0, 8, 8, 8, QColor(35, 35, 35));
        p.fillRect(8, 8, 8, 8, QColor(45, 45, 45));
        p.end();
    }
    
    // Tile the cached pattern
    int startX = rect.left() - (rect.left() % 16);
    int startY = rect.top() - (rect.top() % 16);
    
    for (int y = startY; y < rect.bottom(); y += 16) {
        for (int x = startX; x < rect.right(); x += 16) {
            painter.drawImage(x, y, m_checkerPattern);
        }
    }
}

void CanvasWidget::drawGrid(QPainter& painter, const QRect& rect) {
    painter.setPen(QPen(QColor(80, 80, 80, 60), 1));
    int gridSize = static_cast<int>(50 * m_zoom);
    if (gridSize < 20) gridSize = 20;
    
    for (int x = rect.left(); x <= rect.right(); x += gridSize) {
        painter.drawLine(x, rect.top(), x, rect.bottom());
    }
    for (int y = rect.top(); y <= rect.bottom(); y += gridSize) {
        painter.drawLine(rect.left(), y, rect.right(), y);
    }
}

void CanvasWidget::drawRulers(QPainter& painter) {
    int rulerSize = 20;
    
    // Top ruler
    painter.fillRect(0, 0, width(), rulerSize, QColor(40, 40, 40));
    painter.setPen(QPen(QColor(100, 100, 100), 1));
    painter.drawLine(0, rulerSize - 1, width(), rulerSize - 1);
    
    // Left ruler
    painter.fillRect(0, 0, rulerSize, height(), QColor(40, 40, 40));
    painter.drawLine(rulerSize - 1, 0, rulerSize - 1, height());
    
    // Top ruler markings
    painter.setPen(QColor(130, 130, 130));
    painter.setFont(QFont("Menlo", 7));
    int step = 50;
    for (int x = rulerSize; x < width(); x += step) {
        painter.drawLine(x, rulerSize - 6, x, rulerSize);
        painter.drawText(x + 2, rulerSize - 4, QString::number(x - rulerSize));
    }
    
    // Left ruler markings
    for (int y = rulerSize; y < height(); y += step) {
        painter.drawLine(rulerSize - 6, y, rulerSize, y);
    }
}

void CanvasWidget::drawCompositionBounds(QPainter& painter, const QRect& targetRect) {
    painter.setPen(QPen(QColor(0, 180, 255, 120), 1, Qt::DashLine));
    painter.drawRect(targetRect);
}

void CanvasWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
}

void CanvasWidget::wheelEvent(QWheelEvent* event) {
    double delta = event->angleDelta().y() / 120.0;
    m_zoom = std::clamp(m_zoom + delta * 0.1, 0.1, 10.0);
    emit zoomChanged(m_zoom);
    update();
}

void CanvasWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
        m_dragging = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_offset += QPointF(delta);
        m_lastMousePos = event->pos();
        update();
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
}

void CanvasWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CanvasWidget::dropEvent(QDropEvent* event) {
    const QMimeData* mime = event->mimeData();
    if (mime->hasUrls()) {
        for (const QUrl& url : mime->urls()) {
            if (url.isLocalFile()) {
                emit fileDropped(url.toLocalFile());
            }
        }
    }
}

} // namespace FreeEffect
