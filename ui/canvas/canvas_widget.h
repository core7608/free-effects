#pragma once

#include <QWidget>
#include <QPainter>
#include <QImage>
#include <QRectF>
#include <QPointF>
#include <memory>
#include "../../core/timeline/composition.h"
#include "../../core/rendering/renderer.h"

namespace FreeEffect {

enum class TransformHandle {
    None,
    Move,
    Rotate,
    ScaleTopLeft,
    ScaleTop,
    ScaleTopRight,
    ScaleRight,
    ScaleBottomRight,
    ScaleBottom,
    ScaleBottomLeft,
    ScaleLeft,
    SelectionRect
};

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
    bool isShowGrid() const { return m_showGrid; }
    bool isShowRulers() const { return m_showRulers; }
    
    void setToolMode(const QString& mode) { m_toolMode = mode; }
    void setSelectedLayerIndex(int index) { m_selectedLayerIndex = index; update(); }
    int getSelectedLayerIndex() const { return m_selectedLayerIndex; }
    QRect getSelectedLayerBounds() const { return m_selectedLayerBounds; }

signals:
    void zoomChanged(double zoom);
    void fileDropped(const QString& filePath);
    void layerSelected(int index);
    void layerDeselected();
    void selectionInfoChanged(const QString& info);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void renderFrame();
    void drawCheckerboard(QPainter& painter, const QRect& rect);
    void drawGrid(QPainter& painter, const QRect& rect);
    void drawRulers(QPainter& painter);
    void drawCompositionBounds(QPainter& painter, const QRect& targetRect);
    void drawSelectionRectangle(QPainter& painter);
    void drawLayerBoundingBox(QPainter& painter, const QRect& compRect);
    void drawTransformHandles(QPainter& painter, const QRect& layerRect);
    void drawRotationHandle(QPainter& painter, const QRect& layerRect);
    
    TransformHandle hitTestHandle(const QPoint& pos, const QRect& layerRect) const;
    int hitTestLayer(const QPoint& pos, const QRect& compRect) const;
    QRect getSelectedLayerScreenRect() const;
    QRect getTransformHandleRect(const QPoint& center) const;
    QPointF canvasToComposition(const QPoint& canvasPos) const;
    QPoint compositionToCanvas(const QPointF& compPos) const;
    void updateSelectionInfo();
    
    static constexpr int kHandleSize = 8;
    static constexpr int kRotationHandleRadius = 5;
    static constexpr int kRotationHandleOffset = 25;
    static constexpr double kHandleHalf = kHandleSize / 2.0;
    
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
    QString m_toolMode = "selection";
    
    int m_selectedLayerIndex = -1;
    QRect m_selectedLayerBounds;
    TransformHandle m_activeHandle = TransformHandle::None;
    TransformHandle m_hoverHandle = TransformHandle::None;
    bool m_transformDragging = false;
    QPointF m_transformStartPos;
    QPointF m_transformStartValue;
    QRect m_selectionRect;
    bool m_selectionActive = false;
    QPoint m_selectionStart;
    
    int m_handleSize = kHandleSize;
};

} // namespace FreeEffect
