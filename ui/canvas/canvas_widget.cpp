#include "canvas_widget.h"
#include <QPainter>
#include <QWheelEvent>
#include <QResizeEvent>
#include <cmath>

namespace FreeEffect {

CanvasWidget::CanvasWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(320, 240);
    setAutoFillBackground(true);
    
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
        
        painter.setPen(QColor(80, 80, 80));
        painter.setFont(QFont("Segoe UI", 13));
        painter.drawText(rect(), Qt::AlignCenter, "No composition loaded\n(Ctrl+N to create one)");
        
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

} // namespace FreeEffect
