#pragma once

#include <QWidget>
#include <QPainter>
#include <QImage>
#include "../../core/timeline/composition.h"
#include "../../core/rendering/renderer.h"

namespace FreeEffect {

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget* parent = nullptr);
    ~CanvasWidget() override = default;
    
    void setComposition(std::shared_ptr<Composition> comp);
    void setCurrentTime(double timeInSeconds);
    void setZoom(double zoom);
    void setResolution(int quality);
    void setShowGrid(bool show);
    void setShowRulers(bool show);
    void fitToWindow();
    void zoomIn();
    void zoomOut();
    double getZoom() const { return m_zoom; }

signals:
    void zoomChanged(double zoom);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void renderFrame();
    void drawCheckerboard(QPainter& painter, const QRect& rect);
    void drawGrid(QPainter& painter, const QRect& rect);
    void drawRulers(QPainter& painter);
    void drawCompositionBounds(QPainter& painter, const QRect& targetRect);
    
    std::shared_ptr<Composition> m_composition;
    Renderer m_renderer;
    QImage m_frameImage;
    QImage m_checkerPattern;
    double m_currentTime = 0.0;
    double m_zoom = 1.0;
    bool m_showGrid = false;
    bool m_showRulers = false;
    bool m_dragging = false;
    QPoint m_lastMousePos;
    QPointF m_offset;
};

} // namespace FreeEffect
