#include "timeline_canvas.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

namespace FreeEffect {

TimelineCanvas::TimelineCanvas(QWidget* parent)
    : QWidget(parent) {
    setMouseTracking(true);
    setAutoFillBackground(false);
    setMinimumWidth(400);
    setMinimumHeight(100);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(30, 30, 30));
    setPalette(pal);
}

void TimelineCanvas::setComposition(std::shared_ptr<Composition> comp) {
    m_composition = comp;
    if (comp) {
        m_workAreaEnd = comp->getDuration();
    }
    m_selectedLayer = -1;
    m_scrollX = 0;
    update();
}

void TimelineCanvas::setCurrentTime(double time) {
    m_currentTime = time;
    update();
}

void TimelineCanvas::setPixelsPerSecond(double pps) {
    m_pixelsPerSecond = qBound(10.0, pps, 500.0);
    update();
}

void TimelineCanvas::setSelectedLayer(int index) {
    m_selectedLayer = index;
    update();
}

double TimelineCanvas::xToTime(int x) const {
    return static_cast<double>(x + m_scrollX) / m_pixelsPerSecond;
}

int TimelineCanvas::timeToX(double time) const {
    return static_cast<int>(time * m_pixelsPerSecond) - m_scrollX;
}

void TimelineCanvas::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect fullRect = rect();
    QRect rulerRect(0, 0, width(), RULER_HEIGHT);
    QRect contentRect(0, RULER_HEIGHT, width(), height() - RULER_HEIGHT);

    // Background
    painter.fillRect(fullRect, QColor(30, 30, 30));

    drawAlternatingRows(painter, contentRect);
    drawLayerBars(painter, contentRect);
    drawKeyframes(painter, contentRect);
    drawTimeRuler(painter, rulerRect);
    drawWorkArea(painter, rulerRect);
    drawPlayhead(painter, fullRect);

    // Separator line between ruler and content
    painter.setPen(QPen(QColor(40, 40, 40), 1));
    painter.drawLine(0, RULER_HEIGHT, width(), RULER_HEIGHT);
}

void TimelineCanvas::drawAlternatingRows(QPainter& painter, const QRect& contentRect) {
    if (!m_composition) return;

    int layerCount = m_composition->getLayerCount();
    for (int i = 0; i < layerCount; ++i) {
        int y = contentRect.top() + i * LAYER_ROW_HEIGHT;
        if (y >= contentRect.bottom()) break;

        QRect rowRect(contentRect.left(), y, contentRect.width(), LAYER_ROW_HEIGHT);

        if (i == m_selectedLayer) {
            painter.fillRect(rowRect, QColor(38, 128, 235, 40));
        } else if (i % 2 == 0) {
            painter.fillRect(rowRect, QColor(35, 35, 35));
        } else {
            painter.fillRect(rowRect, QColor(30, 30, 30));
        }

        // Row separator
        painter.setPen(QPen(QColor(40, 40, 40), 1));
        painter.drawLine(rowRect.left(), rowRect.bottom(), rowRect.right(), rowRect.bottom());
    }
}

void TimelineCanvas::drawLayerBars(QPainter& painter, const QRect& contentRect) {
    if (!m_composition) return;

    const auto& layers = m_composition->getLayers();
    double duration = m_composition->getDuration();

    for (int i = 0; i < static_cast<int>(layers.size()); ++i) {
        const auto& layer = layers[i];
        int y = contentRect.top() + i * LAYER_ROW_HEIGHT;
        if (y >= contentRect.bottom()) break;

        int barY = y + 3;
        int barHeight = LAYER_ROW_HEIGHT - 6;

        double layerStart = layer->getStartTime();
        double layerEnd = layerStart + layer->getDuration();
        if (layer->getDuration() <= 0.0) layerEnd = duration;

        int startX = timeToX(layerStart);
        int endX = timeToX(layerEnd);

        // Clamp to visible area
        int visStartX = qMax(startX, contentRect.left());
        int visEndX = qMin(endX, contentRect.right());

        if (visEndX <= visStartX) continue;

        // Color by layer type
        QColor barColor;
        switch (layer->getType()) {
            case LayerType::Video:   barColor = QColor(38, 128, 235); break;
            case LayerType::Audio:   barColor = QColor(46, 204, 113); break;
            case LayerType::Text:    barColor = QColor(230, 126, 34); break;
            case LayerType::Shape:   barColor = QColor(155, 89, 182); break;
            case LayerType::Null:    barColor = QColor(149, 165, 166); break;
            case LayerType::Solid:   barColor = QColor(52, 152, 219); break;
            default:                 barColor = QColor(180, 180, 180); break;
        }

        if (i == m_selectedLayer) {
            barColor = barColor.lighter(130);
        }

        QRect barRect(visStartX, barY, visEndX - visStartX, barHeight);
        QPainterPath path;
        path.addRoundedRect(barRect, 3, 3);
        painter.fillPath(path, barColor);

        // Bar border
        painter.setPen(QPen(barColor.darker(120), 1));
        painter.drawRoundedRect(barRect, 3, 3);

        // Layer name text
        painter.setPen(QColor(255, 255, 255));
        QFont font = painter.font();
        font.setPixelSize(10);
        painter.setFont(font);

        QRect textRect = barRect.adjusted(6, 0, -4, 0);
        QString name = QString::fromStdString(layer->getName());
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, name);
    }
}

void TimelineCanvas::drawTimeRuler(QPainter& painter, const QRect& rulerRect) {
    // Ruler background
    painter.fillRect(rulerRect, QColor(40, 40, 40));

    if (!m_composition) return;

    double duration = m_composition->getDuration();
    double fps = m_composition->getFrameRate().fps;
    if (fps <= 0) fps = 30.0;

    // Determine tick spacing based on zoom
    double secondsPerTick = 1.0;
    if (m_pixelsPerSecond > 120) {
        secondsPerTick = 0.5;
    }
    if (m_pixelsPerSecond > 200) {
        secondsPerTick = 0.25;
    }
    if (m_pixelsPerSecond < 30) {
        secondsPerTick = 2.0;
    }
    if (m_pixelsPerSecond < 15) {
        secondsPerTick = 5.0;
    }

    painter.setPen(QPen(QColor(100, 100, 100), 1));

    QFont font = painter.font();
    font.setPixelSize(9);
    painter.setFont(font);

    double startTime = xToTime(0);
    double endTime = xToTime(width());

    startTime = qMax(startTime, 0.0);
    endTime = qMin(endTime, duration);

    // First tick
    double firstTick = std::floor(startTime / secondsPerTick) * secondsPerTick;

    for (double t = firstTick; t <= endTime; t += secondsPerTick) {
        int x = timeToX(t);
        if (x < rulerRect.left() - 20 || x > rulerRect.right() + 20) continue;

        bool isMajor = (std::fmod(t, secondsPerTick * 4) < 0.001) ||
                       (secondsPerTick >= 1.0 && std::fmod(t, 1.0) < 0.001);

        int tickHeight = isMajor ? 10 : 5;

        painter.setPen(QPen(isMajor ? QColor(160, 160, 160) : QColor(80, 80, 80), 1));
        painter.drawLine(x, rulerRect.bottom() - tickHeight, x, rulerRect.bottom());

        // Time label for major ticks
        if (isMajor) {
            int totalFrames = static_cast<int>(std::round(t * fps));
            int frames = totalFrames % static_cast<int>(fps);
            int seconds = (totalFrames / static_cast<int>(fps)) % 60;
            int minutes = (totalFrames / (static_cast<int>(fps) * 60)) % 60;

            QString label;
            if (minutes > 0 || seconds > 0) {
                label = QString("%1:%2")
                    .arg(minutes, 2, 10, QChar('0'))
                    .arg(seconds, 2, 10, QChar('0'));
            } else {
                label = QString("%1f").arg(frames);
            }

            painter.setPen(QColor(140, 140, 140));
            painter.drawText(x + 3, rulerRect.top() + 10, label);
        }
    }
}

void TimelineCanvas::drawKeyframes(QPainter& painter, const QRect& contentRect) {
    if (!m_composition) return;

    const auto& layers = m_composition->getLayers();
    for (int i = 0; i < static_cast<int>(layers.size()); ++i) {
        const auto& layer = layers[i];
        int y = contentRect.top() + i * LAYER_ROW_HEIGHT;
        if (y >= contentRect.bottom()) break;

        int diamondY = y + LAYER_ROW_HEIGHT / 2;

        auto drawDiamond = [&](double time) {
            int x = timeToX(time);
            if (x < contentRect.left() - 8 || x > contentRect.right() + 8) return;

            QPainterPath diamond;
            diamond.moveTo(x, diamondY - 4);
            diamond.lineTo(x + 4, diamondY);
            diamond.lineTo(x, diamondY + 4);
            diamond.lineTo(x - 4, diamondY);
            diamond.closeSubpath();

            QColor fillColor = (i == m_selectedLayer) ? QColor(255, 200, 0) : QColor(200, 160, 0);
            painter.fillPath(diamond, fillColor);
            painter.setPen(QPen(QColor(180, 140, 0), 1));
            painter.drawPath(diamond);
        };

        // Draw keyframes for transform properties
        for (auto& kf : layer->getPosition().getKeyframes()) {
            drawDiamond(kf.getTime());
        }
        for (auto& kf : layer->getScale().getKeyframes()) {
            drawDiamond(kf.getTime());
        }
        for (auto& kf : layer->getRotation().getKeyframes()) {
            drawDiamond(kf.getTime());
        }
        for (auto& kf : layer->getOpacity().getKeyframes()) {
            drawDiamond(kf.getTime());
        }
    }
}

void TimelineCanvas::drawWorkArea(QPainter& painter, const QRect& rulerRect) {
    int startX = timeToX(m_workAreaStart);
    int endX = timeToX(m_workAreaEnd);

    startX = qMax(startX, rulerRect.left());
    endX = qMin(endX, rulerRect.right());

    if (endX <= startX) return;

    // Work area bracket indicators at top of ruler
    painter.setPen(QPen(QColor(255, 255, 255), 2));

    // Start bracket
    painter.drawLine(startX, rulerRect.top() + 2, startX, rulerRect.top() + 8);
    painter.drawLine(startX, rulerRect.top() + 2, startX + 4, rulerRect.top() + 2);

    // End bracket
    painter.drawLine(endX, rulerRect.top() + 2, endX, rulerRect.top() + 8);
    painter.drawLine(endX, rulerRect.top() + 2, endX - 4, rulerRect.top() + 2);

    // Light work area indicator line at bottom of ruler
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 30));
    painter.drawRect(startX, rulerRect.bottom() - 2, endX - startX, 2);
}

void TimelineCanvas::drawPlayhead(QPainter& painter, const QRect& fullRect) {
    int x = timeToX(m_currentTime);
    if (x < fullRect.left() || x > fullRect.right()) return;

    // Vertical line
    painter.setPen(QPen(QColor(255, 255, 255), 1));
    painter.drawLine(x, fullRect.top(), x, fullRect.bottom());

    // Triangle handle at ruler top
    QPainterPath triangle;
    triangle.moveTo(x - 5, fullRect.top());
    triangle.lineTo(x + 5, fullRect.top());
    triangle.lineTo(x, fullRect.top() + 8);
    triangle.closeSubpath();
    painter.fillPath(triangle, QColor(255, 255, 255));
}

void TimelineCanvas::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        int x = event->position().x();
        int y = event->position().y();

        if (y < RULER_HEIGHT) {
            // Click on ruler - set current time
            double time = xToTime(x);
            if (m_composition) {
                time = qBound(0.0, time, m_composition->getDuration());
            }
            emit timeClicked(time);
        } else {
            // Click on layer area
            int layerIndex = (y - RULER_HEIGHT) / LAYER_ROW_HEIGHT;
            if (m_composition && layerIndex < m_composition->getLayerCount()) {
                emit layerClicked(layerIndex);
            }
        }
    }

    QWidget::mousePressEvent(event);
}

void TimelineCanvas::mouseMoveEvent(QMouseEvent* event) {
    QWidget::mouseMoveEvent(event);
}

void TimelineCanvas::wheelEvent(QWheelEvent* event) {
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom
        double delta = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
        m_pixelsPerSecond = qBound(10.0, m_pixelsPerSecond * delta, 500.0);
        emit zoomChanged(m_pixelsPerSecond);
        update();
    } else if (event->modifiers() & Qt::ShiftModifier) {
        // Horizontal scroll
        m_scrollX -= event->angleDelta().y();
        m_scrollX = qMax(0, m_scrollX);
        update();
    } else {
        QWidget::wheelEvent(event);
    }
}

} // namespace FreeEffect
