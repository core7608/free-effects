#include "canvas_widget.h"
#include <QPainter>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <cmath>
#include <algorithm>

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
    m_selectedLayerIndex = -1;
    m_selectedLayerBounds = QRect();
    m_activeHandle = TransformHandle::None;
    m_hoverHandle = TransformHandle::None;
    m_transformDragging = false;
    m_selectionActive = false;
    m_selectionRect = QRect();
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

QRect CanvasWidget::getSelectedLayerScreenRect() const {
    if (!m_composition || m_selectedLayerIndex < 0) return QRect();
    
    auto layers = m_composition->getLayers();
    if (m_selectedLayerIndex >= static_cast<int>(layers.size())) return QRect();
    
    auto layer = layers[m_selectedLayerIndex];
    if (!layer) return QRect();
    
    int compW = m_composition->getResolution().width;
    int compH = m_composition->getResolution().height;
    
    double scaleX = static_cast<double>(width()) / compW;
    double scaleY = static_cast<double>(height()) / compH;
    double scale = std::min(scaleX, scaleY) * m_zoom * 0.9;
    
    int drawW = static_cast<int>(compW * scale);
    int drawH = static_cast<int>(compH * scale);
    int drawX = (width() - drawW) / 2 + static_cast<int>(m_offset.x());
    int drawY = (height() - drawH) / 2 + static_cast<int>(m_offset.y());
    
    double layerW = compW * 0.5;
    double layerH = compH * 0.5;
    
    double posX = layer->getPosition().getDefaultValue();
    double posY = layer->getPosition().getValueAtTime(m_currentTime);
    if (posX == 0.0 && posY == 0.0) {
        posX = layer->getPosition().getValueAtTime(m_currentTime);
    }
    double scaleVal = layer->getScale().getValueAtTime(m_currentTime);
    double scaledW = layerW * scaleVal / 100.0;
    double scaledH = layerH * scaleVal / 100.0;
    
    int screenW = static_cast<int>(scaledW * scale);
    int screenH = static_cast<int>(scaledH * scale);
    int screenX = drawX + drawW / 2 - screenW / 2 + static_cast<int>(posX * scale);
    int screenY = drawY + drawH / 2 - screenH / 2 + static_cast<int>(posY * scale);
    
    return QRect(screenX, screenY, screenW, screenH);
}

void CanvasWidget::drawSelectionRectangle(QPainter& painter) {
    if (!m_selectionActive) return;
    
    QPen selPen(QColor(0, 212, 255, 150), 1, Qt::DashLine);
    painter.setPen(selPen);
    painter.setBrush(QColor(0, 212, 255, 20));
    painter.drawRect(m_selectionRect);
}

void CanvasWidget::drawLayerBoundingBox(QPainter& painter, const QRect& compRect) {
    if (m_selectedLayerIndex < 0 || !m_composition) return;
    
    auto layers = m_composition->getLayers();
    if (m_selectedLayerIndex >= static_cast<int>(layers.size())) return;
    
    QRect layerRect = getSelectedLayerScreenRect();
    if (layerRect.isNull()) return;
    
    m_selectedLayerBounds = layerRect;
    
    painter.setPen(QPen(QColor(0, 212, 255), 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(layerRect);
}

void CanvasWidget::drawTransformHandles(QPainter& painter, const QRect& layerRect) {
    if (layerRect.isNull()) return;
    
    QColor handleColor(0, 212, 255);
    QColor fillColor(255, 255, 255);
    int half = m_handleSize / 2;
    
    QPoint handles[8];
    handles[0] = layerRect.topLeft();
    handles[1] = QPoint(layerRect.center().x(), layerRect.top());
    handles[2] = layerRect.topRight();
    handles[3] = QPoint(layerRect.right(), layerRect.center().y());
    handles[4] = layerRect.bottomRight();
    handles[5] = QPoint(layerRect.center().x(), layerRect.bottom());
    handles[6] = layerRect.bottomLeft();
    handles[7] = QPoint(layerRect.left(), layerRect.center().y());
    
    for (int i = 0; i < 8; i++) {
        QRect handleRect(handles[i].x() - half, handles[i].y() - half, m_handleSize, m_handleSize);
        
        if (m_hoverHandle != TransformHandle::None) {
            TransformHandle handles_list[8] = {
                TransformHandle::ScaleTopLeft,
                TransformHandle::ScaleTop,
                TransformHandle::ScaleTopRight,
                TransformHandle::ScaleRight,
                TransformHandle::ScaleBottomRight,
                TransformHandle::ScaleBottom,
                TransformHandle::ScaleBottomLeft,
                TransformHandle::ScaleLeft
            };
            if (m_hoverHandle == handles_list[i] || m_activeHandle == handles_list[i]) {
                painter.setPen(QPen(handleColor, 1));
                painter.setBrush(QBrush(handleColor));
                painter.drawRect(handleRect);
                continue;
            }
        }
        
        painter.setPen(QPen(handleColor, 1));
        painter.setBrush(QBrush(fillColor));
        painter.drawRect(handleRect);
    }
}

void CanvasWidget::drawRotationHandle(QPainter& painter, const QRect& layerRect) {
    if (layerRect.isNull()) return;
    
    int centerX = layerRect.center().x();
    int topY = layerRect.top() - kRotationHandleOffset;
    
    painter.setPen(QPen(QColor(0, 212, 255), 1.5));
    painter.drawLine(centerX, layerRect.top(), centerX, topY);
    
    QColor rotColor(0, 212, 255);
    QColor rotFill(255, 255, 255, 180);
    
    if (m_hoverHandle == TransformHandle::Rotate || m_activeHandle == TransformHandle::Rotate) {
        rotFill = QColor(0, 212, 255);
    }
    
    painter.setPen(QPen(rotColor, 1.5));
    painter.setBrush(QBrush(rotFill));
    painter.drawEllipse(QPoint(centerX, topY), kRotationHandleRadius, kRotationHandleRadius);
}

TransformHandle CanvasWidget::hitTestHandle(const QPoint& pos, const QRect& layerRect) const {
    if (layerRect.isNull()) return TransformHandle::None;
    
    int hitRadius = m_handleSize + 2;
    int centerX = layerRect.center().x();
    int topY = layerRect.top() - kRotationHandleOffset;
    QPoint rotCenter(centerX, topY);
    double rotDist = std::hypot(pos.x() - rotCenter.x(), pos.y() - rotCenter.y());
    if (rotDist <= kRotationHandleRadius + 4) {
        return TransformHandle::Rotate;
    }
    
    struct HandleData {
        TransformHandle handle;
        QPoint pos;
    };
    
    HandleData handles[] = {
        {TransformHandle::ScaleTopLeft, layerRect.topLeft()},
        {TransformHandle::ScaleTop, QPoint(layerRect.center().x(), layerRect.top())},
        {TransformHandle::ScaleTopRight, layerRect.topRight()},
        {TransformHandle::ScaleRight, QPoint(layerRect.right(), layerRect.center().y())},
        {TransformHandle::ScaleBottomRight, layerRect.bottomRight()},
        {TransformHandle::ScaleBottom, QPoint(layerRect.center().x(), layerRect.bottom())},
        {TransformHandle::ScaleBottomLeft, layerRect.bottomLeft()},
        {TransformHandle::ScaleLeft, QPoint(layerRect.left(), layerRect.center().y())}
    };
    
    for (const auto& h : handles) {
        double dist = std::hypot(pos.x() - h.pos.x(), pos.y() - h.pos.y());
        if (dist <= hitRadius) {
            return h.handle;
        }
    }
    
    if (layerRect.contains(pos)) {
        return TransformHandle::Move;
    }
    
    return TransformHandle::None;
}

int CanvasWidget::hitTestLayer(const QPoint& pos, const QRect& compRect) const {
    if (!m_composition) return -1;
    
    auto layers = m_composition->getLayers();
    for (int i = static_cast<int>(layers.size()) - 1; i >= 0; i--) {
        if (!layers[i] || !layers[i]->isVisible()) continue;
        
        int compW = m_composition->getResolution().width;
        int compH = m_composition->getResolution().height;
        
        double scaleX = static_cast<double>(width()) / compW;
        double scaleY = static_cast<double>(height()) / compH;
        double scale = std::min(scaleX, scaleY) * m_zoom * 0.9;
        
        int drawW = static_cast<int>(compW * scale);
        int drawH = static_cast<int>(compH * scale);
        int drawX = (width() - drawW) / 2 + static_cast<int>(m_offset.x());
        int drawY = (height() - drawH) / 2 + static_cast<int>(m_offset.y());
        
        double layerW = compW * 0.5;
        double layerH = compH * 0.5;
        
        double posX = layers[i]->getPosition().getDefaultValue();
        double posY = layers[i]->getPosition().getValueAtTime(m_currentTime);
        double scaleVal = layers[i]->getScale().getValueAtTime(m_currentTime);
        double scaledW = layerW * scaleVal / 100.0;
        double scaledH = layerH * scaleVal / 100.0;
        
        int screenW = static_cast<int>(scaledW * scale);
        int screenH = static_cast<int>(scaledH * scale);
        int screenX = drawX + drawW / 2 - screenW / 2 + static_cast<int>(posX * scale);
        int screenY = drawY + drawH / 2 - screenH / 2 + static_cast<int>(posY * scale);
        
        QRect layerScreenRect(screenX, screenY, screenW, screenH);
        if (layerScreenRect.contains(pos)) {
            return i;
        }
    }
    
    return -1;
}

QRect CanvasWidget::getTransformHandleRect(const QPoint& center) const {
    int half = m_handleSize / 2;
    return QRect(center.x() - half, center.y() - half, m_handleSize, m_handleSize);
}

QPointF CanvasWidget::canvasToComposition(const QPoint& canvasPos) const {
    if (!m_composition) return QPointF();
    
    int compW = m_composition->getResolution().width;
    int compH = m_composition->getResolution().height;
    
    double scaleX = static_cast<double>(width()) / compW;
    double scaleY = static_cast<double>(height()) / compH;
    double scale = std::min(scaleX, scaleY) * m_zoom * 0.9;
    
    int drawW = static_cast<int>(compW * scale);
    int drawH = static_cast<int>(compH * scale);
    int drawX = (width() - drawW) / 2 + static_cast<int>(m_offset.x());
    int drawY = (height() - drawH) / 2 + static_cast<int>(m_offset.y());
    
    double compX = (canvasPos.x() - drawX - drawW / 2.0) / scale;
    double compY = (canvasPos.y() - drawY - drawH / 2.0) / scale;
    
    return QPointF(compX, compY);
}

QPoint CanvasWidget::compositionToCanvas(const QPointF& compPos) const {
    if (!m_composition) return QPoint();
    
    int compW = m_composition->getResolution().width;
    int compH = m_composition->getResolution().height;
    
    double scaleX = static_cast<double>(width()) / compW;
    double scaleY = static_cast<double>(height()) / compH;
    double scale = std::min(scaleX, scaleY) * m_zoom * 0.9;
    
    int drawW = static_cast<int>(compW * scale);
    int drawH = static_cast<int>(compH * scale);
    int drawX = (width() - drawW) / 2 + static_cast<int>(m_offset.x());
    int drawY = (height() - drawH) / 2 + static_cast<int>(m_offset.y());
    
    int canvasX = drawX + drawW / 2 + static_cast<int>(compPos.x() * scale);
    int canvasY = drawY + drawH / 2 + static_cast<int>(compPos.y() * scale);
    
    return QPoint(canvasX, canvasY);
}

void CanvasWidget::updateSelectionInfo() {
    if (m_selectedLayerIndex < 0 || !m_composition) {
        emit selectionInfoChanged("");
        return;
    }
    
    auto layers = m_composition->getLayers();
    if (m_selectedLayerIndex >= static_cast<int>(layers.size())) return;
    
    auto layer = layers[m_selectedLayerIndex];
    double posX = layer->getPosition().getDefaultValue();
    double posY = layer->getPosition().getValueAtTime(m_currentTime);
    double scaleVal = layer->getScale().getValueAtTime(m_currentTime);
    double rotVal = layer->getRotation().getValueAtTime(m_currentTime);
    
    QString info = QString("Pos: %1, %2  |  Scale: %3%  |  Rot: %4%")
        .arg(posX, 0, 'f', 1)
        .arg(posY, 0, 'f', 1)
        .arg(scaleVal, 0, 'f', 1)
        .arg(rotVal, 0, 'f', 1);
    
    emit selectionInfoChanged(info);
}

void CanvasWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);
    
    painter.fillRect(rect(), QColor(25, 25, 25));
    
    if (!m_composition) {
        drawCheckerboard(painter, rect());
        
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
    
    drawCheckerboard(painter, targetRect);
    
    if (!m_frameImage.isNull()) {
        painter.drawImage(targetRect, m_frameImage);
    }
    
    painter.setPen(QPen(QColor(60, 60, 60), 1));
    painter.drawRect(targetRect);
    
    QPen dashPen(QColor(100, 100, 100, 100), 1, Qt::DashLine);
    painter.setPen(dashPen);
    int margin50W = drawW / 10;
    int margin50H = drawH / 10;
    painter.drawRect(targetRect.adjusted(margin50W, margin50H, -margin50W, -margin50H));
    
    int margin90W = drawW / 20;
    int margin90H = drawH / 20;
    painter.drawRect(targetRect.adjusted(margin90W, margin90H, -margin90W, -margin90H));
    
    if (m_showGrid) drawGrid(painter, targetRect);
    
    drawSelectionRectangle(painter);
    
    drawLayerBoundingBox(painter, targetRect);
    
    if (m_selectedLayerIndex >= 0) {
        QRect layerRect = getSelectedLayerScreenRect();
        drawTransformHandles(painter, layerRect);
        drawRotationHandle(painter, layerRect);
    }
    
    if (m_showRulers) drawRulers(painter);
    
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
    if (m_checkerPattern.isNull() || m_checkerPattern.width() != 16) {
        m_checkerPattern = QImage(16, 16, QImage::Format_RGB32);
        QPainter p(&m_checkerPattern);
        p.fillRect(0, 0, 8, 8, QColor(45, 45, 45));
        p.fillRect(8, 0, 8, 8, QColor(35, 35, 35));
        p.fillRect(0, 8, 8, 8, QColor(35, 35, 35));
        p.fillRect(8, 8, 8, 8, QColor(45, 45, 45));
        p.end();
    }
    
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
    
    painter.fillRect(0, 0, width(), rulerSize, QColor(40, 40, 40));
    painter.setPen(QPen(QColor(100, 100, 100), 1));
    painter.drawLine(0, rulerSize - 1, width(), rulerSize - 1);
    
    painter.fillRect(0, 0, rulerSize, height(), QColor(40, 40, 40));
    painter.drawLine(rulerSize - 1, 0, rulerSize - 1, height());
    
    painter.setPen(QColor(130, 130, 130));
    painter.setFont(QFont("Menlo", 7));
    int step = 50;
    for (int x = rulerSize; x < width(); x += step) {
        painter.drawLine(x, rulerSize - 6, x, rulerSize);
        painter.drawText(x + 2, rulerSize - 4, QString::number(x - rulerSize));
    }
    
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
        return;
    }
    
    if (event->button() != Qt::LeftButton) return;
    
    QPoint pos = event->pos();
    
    if (m_toolMode == "selection" || m_toolMode.isEmpty()) {
        QRect layerRect = getSelectedLayerScreenRect();
        TransformHandle handle = hitTestHandle(pos, layerRect);
        
        if (handle != TransformHandle::None && handle != TransformHandle::Move) {
            m_activeHandle = handle;
            m_transformDragging = true;
            m_transformStartPos = pos;
            
            if (m_selectedLayerIndex >= 0 && m_composition) {
                auto layers = m_composition->getLayers();
                if (m_selectedLayerIndex < static_cast<int>(layers.size())) {
                    auto layer = layers[m_selectedLayerIndex];
                    if (handle == TransformHandle::Rotate) {
                        m_transformStartValue = QPointF(
                            layer->getRotation().getDefaultValue(), 0);
                    } else {
                        m_transformStartValue = QPointF(
                            layer->getPosition().getDefaultValue(),
                            layer->getScale().getDefaultValue());
                    }
                }
            }
            update();
            return;
        }
        
        if (handle == TransformHandle::Move) {
            m_activeHandle = TransformHandle::Move;
            m_transformDragging = true;
            m_transformStartPos = pos;
            if (m_selectedLayerIndex >= 0 && m_composition) {
                auto layers = m_composition->getLayers();
                if (m_selectedLayerIndex < static_cast<int>(layers.size())) {
                    auto layer = layers[m_selectedLayerIndex];
                    m_transformStartValue = QPointF(
                        layer->getPosition().getDefaultValue(),
                        layer->getPosition().getValueAtTime(m_currentTime));
                }
            }
            update();
            return;
        }
        
        int compW = m_composition ? m_composition->getResolution().width : 0;
        int compH = m_composition ? m_composition->getResolution().height : 0;
        double scale = 1.0;
        if (compW > 0 && compH > 0) {
            double sx = static_cast<double>(width()) / compW;
            double sy = static_cast<double>(height()) / compH;
            scale = std::min(sx, sy) * m_zoom * 0.9;
        }
        int drawW = static_cast<int>(compW * scale);
        int drawH = static_cast<int>(compH * scale);
        int drawX = (width() - drawW) / 2 + static_cast<int>(m_offset.x());
        int drawY = (height() - drawH) / 2 + static_cast<int>(m_offset.y());
        QRect targetRect(drawX, drawY, drawW, drawH);
        
        int layerIdx = hitTestLayer(pos, targetRect);
        if (layerIdx >= 0) {
            m_selectedLayerIndex = layerIdx;
            emit layerSelected(layerIdx);
            updateSelectionInfo();
        } else {
            m_selectedLayerIndex = -1;
            m_selectedLayerBounds = QRect();
            emit layerDeselected();
            emit selectionInfoChanged("");
            
            m_selectionActive = true;
            m_selectionStart = pos;
            m_selectionRect = QRect(pos, pos);
        }
        
        update();
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    QPoint pos = event->pos();
    
    if (m_dragging) {
        QPoint delta = pos - m_lastMousePos;
        m_offset += QPointF(delta);
        m_lastMousePos = pos;
        update();
        return;
    }
    
    if (m_transformDragging && m_activeHandle != TransformHandle::None) {
        QPoint delta = pos - m_transformStartPos.toPoint();
        
        if (!m_composition || m_selectedLayerIndex < 0) return;
        
        auto layers = m_composition->getLayers();
        if (m_selectedLayerIndex >= static_cast<int>(layers.size())) return;
        
        auto layer = layers[m_selectedLayerIndex];
        
        int compW = m_composition->getResolution().width;
        int compH = m_composition->getResolution().height;
        double baseScale = 1.0;
        if (compW > 0 && compH > 0) {
            double sx = static_cast<double>(width()) / compW;
            double sy = static_cast<double>(height()) / compH;
            baseScale = std::min(sx, sy) * m_zoom * 0.9;
        }
        
        double compDeltaX = delta.x() / baseScale;
        double compDeltaY = delta.y() / baseScale;
        
        if (m_activeHandle == TransformHandle::Move) {
            double newX = m_transformStartValue.x() + compDeltaX;
            double newY = m_transformStartValue.y() + compDeltaY;
            layer->getPosition().setDefaultValue(newX);
        } else if (m_activeHandle == TransformHandle::Rotate) {
            QRect layerRect = getSelectedLayerScreenRect();
            QPointF center = QPointF(layerRect.center());
            double angle = std::atan2(pos.y() - center.y(), pos.x() - center.x());
            double startAngle = std::atan2(
                m_transformStartPos.y() - center.y(),
                m_transformStartPos.x() - center.x());
            double angleDelta = (angle - startAngle) * 180.0 / M_PI;
            double newRot = m_transformStartValue.x() + angleDelta;
            layer->getRotation().setDefaultValue(newRot);
        } else {
            double scaleDelta = 0.0;
            switch (m_activeHandle) {
                case TransformHandle::ScaleTopLeft:
                case TransformHandle::ScaleTopRight:
                case TransformHandle::ScaleBottomLeft:
                case TransformHandle::ScaleBottomRight:
                    scaleDelta = (compDeltaX + compDeltaY) * 0.5;
                    break;
                case TransformHandle::ScaleTop:
                case TransformHandle::ScaleBottom:
                    scaleDelta = compDeltaY;
                    break;
                case TransformHandle::ScaleLeft:
                case TransformHandle::ScaleRight:
                    scaleDelta = compDeltaX;
                    break;
                default:
                    break;
            }
            
            double origScale = m_transformStartValue.y();
            double newScale = std::max(0.1, origScale + scaleDelta);
            layer->getScale().setDefaultValue(newScale);
        }
        
        updateSelectionInfo();
        update();
        return;
    }
    
    if (m_selectionActive) {
        m_selectionRect = QRect(m_selectionStart, pos).normalized();
        update();
        return;
    }
    
    if (m_toolMode == "selection" || m_toolMode.isEmpty()) {
        QRect layerRect = getSelectedLayerScreenRect();
        TransformHandle handle = hitTestHandle(pos, layerRect);
        
        if (handle != m_hoverHandle) {
            m_hoverHandle = handle;
            update();
        }
        
        if (handle == TransformHandle::Move) {
            setCursor(Qt::SizeAllCursor);
        } else if (handle == TransformHandle::Rotate) {
            setCursor(Qt::CrossCursor);
        } else if (handle != TransformHandle::None) {
            Qt::CursorShape cursors[] = {
                Qt::SizeFDiagCursor,
                Qt::SizeVerCursor,
                Qt::SizeBDiagCursor,
                Qt::SizeHorCursor,
                Qt::SizeFDiagCursor,
                Qt::SizeVerCursor,
                Qt::SizeBDiagCursor,
                Qt::SizeHorCursor
            };
            int idx = static_cast<int>(handle) - static_cast<int>(TransformHandle::ScaleTopLeft);
            if (idx >= 0 && idx < 8) {
                setCursor(cursors[idx]);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        } else {
            int compW = m_composition ? m_composition->getResolution().width : 0;
            int compH = m_composition ? m_composition->getResolution().height : 0;
            double scale = 1.0;
            if (compW > 0 && compH > 0) {
                double sx = static_cast<double>(width()) / compW;
                double sy = static_cast<double>(height()) / compH;
                scale = std::min(sx, sy) * m_zoom * 0.9;
            }
            int drawW = static_cast<int>(compW * scale);
            int drawH = static_cast<int>(compH * scale);
            int drawX = (width() - drawW) / 2 + static_cast<int>(m_offset.x());
            int drawY = (height() - drawH) / 2 + static_cast<int>(m_offset.y());
            QRect targetRect(drawX, drawY, drawW, drawH);
            
            int layerIdx = hitTestLayer(pos, targetRect);
            if (layerIdx >= 0) {
                setCursor(Qt::PointingHandCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        if (m_selectionActive) {
            m_selectionActive = false;
            m_selectionRect = QRect();
            update();
        }
        
        if (m_transformDragging) {
            m_transformDragging = false;
            m_activeHandle = TransformHandle::None;
            update();
        }
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
