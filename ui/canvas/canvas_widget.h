#pragma once

#include <QWidget>
#include <QPainter>
#include <QImage>
#include <QRectF>
#include <QPointF>
#include <QTimer>
#include <QMutex>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include "../../core/timeline/composition.h"
#include "../../core/timeline/shape_operations.h"
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

enum class ViewMode {
    ActiveCamera, CustomView1, CustomView2, CustomView3,
    Top, Front, Right, Left, Back, Bottom
};

enum class ChannelDisplay { RGB, Red, Green, Blue, Alpha };

enum class PreviewResolution { Auto, Full, Half, Third, Quarter };

class MultiFrameRenderer {
public:
    MultiFrameRenderer();
    ~MultiFrameRenderer();

    void setThreadCount(int count);
    int getThreadCount() const { return m_threadCount; }

    using ProgressCallback = std::function<void(double progress)>;
    using FrameCallback = std::function<void(int frame, const QImage& image)>;

    void renderFrameRange(Composition& comp, int startFrame, int endFrame,
                          double fps, int width, int height);
    void cancel();
    bool isRendering() const { return m_rendering.load(); }
    double getProgress() const { return m_progress.load(); }

    void setProgressCallback(ProgressCallback cb) { m_progressCallback = std::move(cb); }
    void setFrameCallback(FrameCallback cb) { m_frameCallback = std::move(cb); }

    const QImage& getFrame(int index) const;

private:
    void workerThread(Composition& comp, int startFrame, int endFrame,
                      double fps, int width, int height,
                      int threadId, int totalThreads);

    int m_threadCount = 4;
    std::atomic<bool> m_rendering{false};
    std::atomic<bool> m_cancelRequested{false};
    std::atomic<double> m_progress{0.0};
    std::vector<std::thread> m_threads;
    mutable QMutex m_mutex;
    std::vector<QImage> m_renderedFrames;

    ProgressCallback m_progressCallback;
    FrameCallback m_frameCallback;
    int m_totalFrames = 0;
    std::atomic<int> m_framesCompleted{0};
};

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget* parent = nullptr);
    ~CanvasWidget() override;

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

    // RAM Preview
    void startRAMPreview();
    void stopRAMPreview();
    void stepForward();
    void stepBackward();
    void goToFirstFrame();
    void goToLastFrame();
    bool isRAMPreviewing() const { return m_ramPreviewActive; }
    bool hasRAMPreviewFrames() const { return !m_ramPreviewFrames.empty(); }

    // Multi-View
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const { return m_viewMode; }
    void setMultiViewEnabled(bool enabled);
    bool isMultiViewEnabled() const { return m_multiViewEnabled; }
    void setViewCount(int count);
    int getViewCount() const { return m_viewCount; }

    // Channel Display
    void setChannelDisplay(ChannelDisplay channel);
    ChannelDisplay getChannelDisplay() const { return m_channelDisplay; }

    // Safe Margins
    void setShowSafeMargins(bool show);
    bool isShowSafeMargins() const { return m_showSafeMargins; }
    void setSafeMarginsPercentage(double title, double action);

    // Region of Interest
    void setROIEnabled(bool enabled);
    bool isROIEnabled() const { return m_roiEnabled; }
    void setROI(int x, int y, int w, int h);

    // Preview Resolution
    void setPreviewResolution(PreviewResolution res);
    PreviewResolution getPreviewResolution() const { return m_previewResolution; }

    // Guides
    void addHorizontalGuide(double position);
    void addVerticalGuide(double position);
    void removeGuide(int index);
    void clearGuides();
    bool isSnapToGuides() const { return m_snapToGuides; }
    void setSnapToGuides(bool snap);

    // Multi-Frame Renderer access
    MultiFrameRenderer& getMultiFrameRenderer() { return m_multiFrameRenderer; }

signals:
    void zoomChanged(double zoom);
    void fileDropped(const QString& filePath);
    void layerSelected(int index);
    void layerDeselected();
    void selectionInfoChanged(const QString& info);
    void currentTimeChanged(double time);
    void ramPreviewFinished();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
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

    // New drawing methods
    void drawSafeMargins(QPainter& painter, const QRect& targetRect);
    void drawROI(QPainter& painter, const QRect& targetRect);
    void drawMotionPath(QPainter& painter, const QRect& compRect);
    void drawGuides(QPainter& painter, const QRect& targetRect);
    void drawChannelOverlay(QPainter& painter, const QRect& targetRect);
    void drawMultiView(QPainter& painter, const QRect& targetRect);
    void drawPreviewResolutionLabel(QPainter& painter);

    // Coordinate mapping
    QImage applyChannelDisplay(const QImage& source) const;
    double getPreviewScaleFactor() const;
    QRect applyROIClip(const QRect& targetRect) const;

    // Snap
    Point2D snapToGuides(Point2D point) const;
    double snapToGrid(double value, double gridSize) const;

    TransformHandle hitTestHandle(const QPoint& pos, const QRect& layerRect) const;
    int hitTestLayer(const QPoint& pos, const QRect& compRect) const;
    QRect getSelectedLayerScreenRect() const;
    QRect getTransformHandleRect(const QPoint& center) const;
    QPointF canvasToComposition(const QPoint& canvasPos) const;
    QPoint compositionToCanvas(const QPointF& compPos) const;
    void updateSelectionInfo();

    void onRAMPreviewTimer();
    int getTotalFrames() const;

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

    // RAM Preview
    bool m_ramPreviewActive = false;
    std::vector<QImage> m_ramPreviewFrames;
    int m_ramPreviewStartFrame = 0;
    int m_ramPreviewEndFrame = 0;
    int m_ramPreviewCurrentFrame = 0;
    QTimer* m_ramPreviewTimer = nullptr;

    // Multi-View
    ViewMode m_viewMode = ViewMode::ActiveCamera;
    bool m_multiViewEnabled = false;
    int m_viewCount = 2;

    // Channel Display
    ChannelDisplay m_channelDisplay = ChannelDisplay::RGB;

    // Safe Margins
    bool m_showSafeMargins = false;
    double m_titleSafePercent = 10.0;
    double m_actionSafePercent = 5.0;

    // ROI
    bool m_roiEnabled = false;
    QRect m_roiRect;

    // Preview Resolution
    PreviewResolution m_previewResolution = PreviewResolution::Auto;

    // Guides
    std::vector<double> m_horizontalGuides;
    std::vector<double> m_verticalGuides;
    bool m_snapToGuides = false;
    static constexpr double kSnapThreshold = 5.0;

    // Multi-Frame Renderer
    MultiFrameRenderer m_multiFrameRenderer;
};

} // namespace FreeEffect
