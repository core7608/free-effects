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
    pal.setColor(QPalette::Window, QColor(30, 30, 30));
    setPalette(pal);
    setMouseTracking(true);
}

void CanvasWidget::setComposition(std::shared_ptr<Composition> comp) {
    m_composition = comp;
    m_frameImage = QImage();
    update();
}

void CanvasWidget::setCurrentTime(double timeInSeconds) {
    m_currentTime = timeInSeconds;
    renderFrame();
    update();
}

void CanvasWidget::setZoom(double zoom) {
    m_zoom = zoom;
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
    
    m_frameImage = QImage(buffer.width, buffer.height, QImage::Format_RGBA8888);
    memcpy(m_frameImage.bits(), buffer.data.data(), buffer.data.size());
}

void CanvasWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    drawCheckerboard(painter, rect());
    
    if (!m_composition) {
        painter.setPen(QColor(120, 120, 120));
        painter.setFont(QFont("Arial", 14));
        painter.drawText(rect(), Qt::AlignCenter, "No composition loaded\n(Ctrl+N to create one)");
        return;
    }
    
    if (!m_frameImage.isNull()) {
        int compW = m_composition->getResolution().width;
        int compH = m_composition->getResolution().height;
        
        double scaleX = static_cast<double>(width()) / compW;
        double scaleY = static_cast<double>(height()) / compH;
        double scale = std::min(scaleX, scaleY) * m_zoom;
        
        int drawW = static_cast<int>(compW * scale);
        int drawH = static_cast<int>(compH * scale);
        int drawX = (width() - drawW) / 2 + static_cast<int>(m_offset.x());
        int drawY = (height() - drawH) / 2 + static_cast<int>(m_offset.y());
        
        QRect targetRect(drawX, drawY, drawW, drawH);
        painter.drawImage(targetRect, m_frameImage);
        
        painter.setPen(QColor(80, 80, 80));
        painter.drawRect(targetRect);
        
        if (m_showGrid) drawGrid(painter, targetRect);
        if (m_showRulers) drawRulers(painter);
    }
}

void CanvasWidget::drawCheckerboard(QPainter& painter, const QRect& rect) {
    int tileSize = 8;
    for (int y = rect.top(); y < rect.bottom(); y += tileSize) {
        for (int x = rect.left(); x < rect.right(); x += tileSize) {
            bool light = ((x / tileSize) + (y / tileSize)) % 2 == 0;
            painter.fillRect(x, y, tileSize, tileSize, light ? QColor(50, 50, 50) : QColor(40, 40, 40));
        }
    }
}

void CanvasWidget::drawGrid(QPainter& painter, const QRect& rect) {
    painter.setPen(QColor(100, 100, 100, 80));
    int gridSize = 50;
    
    for (int x = rect.left(); x <= rect.right(); x += gridSize) {
        painter.drawLine(x, rect.top(), x, rect.bottom());
    }
    for (int y = rect.top(); y <= rect.bottom(); y += gridSize) {
        painter.drawLine(rect.left(), y, rect.right(), y);
    }
}

void CanvasWidget::drawRulers(QPainter& painter) {
    painter.setPen(QColor(180, 180, 180));
    painter.fillRect(0, 0, width(), 20, QColor(60, 60, 60));
    painter.fillRect(0, 0, 20, height(), QColor(60, 60, 60));
    
    painter.setFont(QFont("Monospace", 7));
    for (int x = 20; x < width(); x += 50) {
        painter.drawLine(x, 0, x, 10);
        painter.drawText(x + 2, 9, QString::number(x - 20));
    }
    for (int y = 20; y < height(); y += 50) {
        painter.drawLine(0, y, 10, y);
    }
}

void CanvasWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
}

void CanvasWidget::wheelEvent(QWheelEvent* event) {
    double delta = event->angleDelta().y() / 120.0;
    m_zoom = std::clamp(m_zoom + delta * 0.1, 0.1, 10.0);
    update();
}

void CanvasWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
        m_dragging = true;
        m_lastMousePos = event->pos();
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
    }
}

} // namespace FreeEffect
