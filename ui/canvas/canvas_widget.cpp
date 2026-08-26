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

// ── MultiFrameRenderer ──────────────────────────────────────────

MultiFrameRenderer::MultiFrameRenderer() = default;

MultiFrameRenderer::~MultiFrameRenderer() {
    cancel();
    for (auto& t : m_threads) {
        if (t.joinable()) t.join();
    }
}

void MultiFrameRenderer::setThreadCount(int count) {
    m_threadCount = std::max(1, std::min(count, 32));
}

void MultiFrameRenderer::renderFrameRange(
    Composition& comp, int startFrame, int endFrame,
    double fps, int width, int height) {

    if (m_rendering.load()) return;

    m_cancelRequested = false;
    m_framesCompleted = 0;
    m_totalFrames = endFrame - startFrame + 1;
    m_progress = 0.0;

    {
        QMutexLocker lock(&m_mutex);
        m_renderedFrames.clear();
        m_renderedFrames.resize(m_totalFrames);
    }

    m_rendering = true;

    int totalThreads = std::min(m_threadCount, m_totalFrames);
    m_threads.clear();
    m_threads.reserve(totalThreads);

    for (int i = 0; i < totalThreads; i++) {
        m_threads.emplace_back(&MultiFrameRenderer::workerThread, this,
                               std::ref(comp), startFrame, endFrame,
                               fps, width, height, i, totalThreads);
    }

    for (auto& t : m_threads) {
        if (t.joinable()) t.join();
    }

    m_threads.clear();
    m_rendering = false;
    m_progress = 1.0;
}

void MultiFrameRenderer::cancel() {
    m_cancelRequested = true;
}

const QImage& MultiFrameRenderer::getFrame(int index) const {
    QMutexLocker lock(&m_mutex);
    static QImage empty;
    if (index < 0 || index >= static_cast<int>(m_renderedFrames.size())) {
        return empty;
    }
    return m_renderedFrames[index];
}

void MultiFrameRenderer::workerThread(
    Composition& comp, int startFrame, int endFrame,
    double fps, int width, int height,
    int threadId, int totalThreads) {

    Renderer renderer;
    renderer.setQuality(100);
    renderer.setResolution(width, height);

    int totalFrames = endFrame - startFrame + 1;

    for (int frame = threadId; frame < totalFrames; frame += totalThreads) {
        if (m_cancelRequested.load()) break;

        int actualFrame = startFrame + frame;
        double time = static_cast<double>(actualFrame) / fps;

        PixelBuffer buffer = renderer.renderFrame(comp, time);

        QImage img;
        if (buffer.width > 0 && buffer.height > 0 &&
            buffer.data.size() >= static_cast<size_t>(buffer.width * buffer.height * 4)) {
            img = QImage(buffer.width, buffer.height, QImage::Format_RGBA8888);
            memcpy(img.bits(), buffer.data.data(), buffer.data.size());
        }

        {
            QMutexLocker lock(&m_mutex);
            if (frame < static_cast<int>(m_renderedFrames.size())) {
                m_renderedFrames[frame] = img;
            }
        }

        int completed = m_framesCompleted.fetch_add(1) + 1;
        m_progress.store(static_cast<double>(completed) / totalFrames);

        if (m_frameCallback) {
            m_frameCallback(actualFrame, img);
        }
        if (m_progressCallback) {
            m_progressCallback(m_progress.load());
        }
    }
}

// ── CanvasWidget ────────────────────────────────────────────────

CanvasWidget::CanvasWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(320, 240);
    setAutoFillBackground(true);
    setAcceptDrops(true);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(25, 25, 25));
    setPalette(pal);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_ramPreviewTimer = new QTimer(this);
    connect(m_ramPreviewTimer, &QTimer::timeout, this, &CanvasWidget::onRAMPreviewTimer);
}

CanvasWidget::~CanvasWidget() {
    stopRAMPreview();
}

void CanvasWidget::setComposition(std::shared_ptr<Composition> comp) {
    stopRAMPreview();
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
    m_ramPreviewFrames.clear();
    update();
}

void CanvasWidget::setCurrentTime(double timeInSeconds) {
    m_currentTime = timeInSeconds;
    renderFrame();
    update();
    emit currentTimeChanged(timeInSeconds);
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

// ── RAM Preview ─────────────────────────────────────────────────

void CanvasWidget::startRAMPreview() {
    if (!m_composition || m_ramPreviewActive) return;

    m_ramPreviewActive = true;
    m_ramPreviewStartFrame = static_cast<int>(m_composition->getWorkAreaStart() * m_composition->getFrameRate().fps);
    m_ramPreviewEndFrame = static_cast<int>(m_composition->getWorkAreaEnd() * m_composition->getFrameRate().fps);

    if (m_ramPreviewEndFrame <= m_ramPreviewStartFrame) {
        m_ramPreviewEndFrame = static_cast<int>(m_composition->getDuration() * m_composition->getFrameRate().fps);
    }

    int totalFrames = m_ramPreviewEndFrame - m_ramPreviewStartFrame;
    if (totalFrames <= 0) {
        m_ramPreviewActive = false;
        return;
    }

    m_ramPreviewFrames.clear();
    m_ramPreviewFrames.resize(totalFrames);

    int compW = m_composition->getResolution().width;
    int compH = m_composition->getResolution().height;
    double fps = m_composition->getFrameRate().fps;

    m_multiFrameRenderer.setThreadCount(std::thread::hardware_concurrency());
    m_multiFrameRenderer.renderFrameRange(
        *m_composition, m_ramPreviewStartFrame, m_ramPreviewEndFrame,
        fps, compW, compH);

    for (int i = 0; i < totalFrames; i++) {
        m_ramPreviewFrames[i] = m_multiFrameRenderer.getFrame(i);
    }

    m_ramPreviewCurrentFrame = 0;
    m_currentTime = static_cast<double>(m_ramPreviewStartFrame) / fps;

    int intervalMs = static_cast<int>(1000.0 / fps);
    m_ramPreviewTimer->start(intervalMs);

    update();
}

void CanvasWidget::stopRAMPreview() {
    if (m_ramPreviewTimer) m_ramPreviewTimer->stop();
    m_multiFrameRenderer.cancel();
    m_ramPreviewActive = false;
    m_ramPreviewFrames.clear();
}

void CanvasWidget::stepForward() {
    if (!m_composition) return;
    double frameDur = m_composition->getFrameRate().getFrameDuration();
    m_currentTime += frameDur;
    double maxTime = m_composition->getDuration();
    if (m_currentTime > maxTime) m_currentTime = maxTime;
    renderFrame();
    update();
    emit currentTimeChanged(m_currentTime);
}

void CanvasWidget::stepBackward() {
    if (!m_composition) return;
    double frameDur = m_composition->getFrameRate().getFrameDuration();
    m_currentTime -= frameDur;
    if (m_currentTime < 0.0) m_currentTime = 0.0;
    renderFrame();
    update();
    emit currentTimeChanged(m_currentTime);
}

void CanvasWidget::goToFirstFrame() {
    if (!m_composition) return;
    m_currentTime = m_composition->getWorkAreaStart();
    renderFrame();
    update();
    emit currentTimeChanged(m_currentTime);
}

void CanvasWidget::goToLastFrame() {
    if (!m_composition) return;
    m_currentTime = m_composition->getWorkAreaEnd();
    if (m_currentTime <= 0.0) m_currentTime = m_composition->getDuration();
    renderFrame();
    update();
    emit currentTimeChanged(m_currentTime);
}

void CanvasWidget::onRAMPreviewTimer() {
    if (!m_ramPreviewActive || m_ramPreviewFrames.empty()) {
        stopRAMPreview();
        emit ramPreviewFinished();
        return;
    }

    if (m_ramPreviewCurrentFrame >= static_cast<int>(m_ramPreviewFrames.size())) {
        m_ramPreviewCurrentFrame = 0;
    }

    m_frameImage = m_ramPreviewFrames[m_ramPreviewCurrentFrame];
    m_ramPreviewCurrentFrame++;

    if (m_composition) {
        double fps = m_composition->getFrameRate().fps;
        m_currentTime = static_cast<double>(m_ramPreviewStartFrame + m_ramPreviewCurrentFrame) / fps;
    }

    update();
    emit currentTimeChanged(m_currentTime);
}

int CanvasWidget::getTotalFrames() const {
    if (!m_composition) return 0;
    return static_cast<int>(m_composition->getDuration() * m_composition->getFrameRate().fps);
}

// ── Multi-View ──────────────────────────────────────────────────

void CanvasWidget::setViewMode(ViewMode mode) {
    m_viewMode = mode;
    renderFrame();
    update();
}

void CanvasWidget::setMultiViewEnabled(bool enabled) {
    m_multiViewEnabled = enabled;
    update();
}

void CanvasWidget::setViewCount(int count) {
    m_viewCount = std::clamp(count, 1, 4);
    update();
}

// ── Channel Display ─────────────────────────────────────────────

void CanvasWidget::setChannelDisplay(ChannelDisplay channel) {
    m_channelDisplay = channel;
    update();
}

QImage CanvasWidget::applyChannelDisplay(const QImage& source) const {
    if (m_channelDisplay == ChannelDisplay::RGB || source.isNull()) {
        return source;
    }

    QImage result(source.size(), QImage::Format_RGBA8888);
    result.fill(QColor(0, 0, 0, 255));

    for (int y = 0; y < source.height(); y++) {
        for (int x = 0; x < source.width(); x++) {
            QColor c = source.pixelColor(x, y);
            uint8_t* dst = result.scanLine(y) + x * 4;

            switch (m_channelDisplay) {
                case ChannelDisplay::Red:
                    dst[0] = c.red();
                    dst[1] = c.red();
                    dst[2] = c.red();
                    dst[3] = 255;
                    break;
                case ChannelDisplay::Green:
                    dst[0] = c.green();
                    dst[1] = c.green();
                    dst[2] = c.green();
                    dst[3] = 255;
                    break;
                case ChannelDisplay::Blue:
                    dst[0] = c.blue();
                    dst[1] = c.blue();
                    dst[2] = c.blue();
                    dst[3] = 255;
                    break;
                case ChannelDisplay::Alpha:
                    dst[0] = c.alpha();
                    dst[1] = c.alpha();
                    dst[2] = c.alpha();
                    dst[3] = 255;
                    break;
                default:
                    break;
            }
        }
    }
    return result;
}

// ── Safe Margins ────────────────────────────────────────────────

void CanvasWidget::setShowSafeMargins(bool show) {
    m_showSafeMargins = show;
    update();
}

void CanvasWidget::setSafeMarginsPercentage(double title, double action) {
    m_titleSafePercent = title;
    m_actionSafePercent = action;
    update();
}

void CanvasWidget::drawSafeMargins(QPainter& painter, const QRect& targetRect) {
    if (!m_showSafeMargins) return;

    int titleW = static_cast<int>(targetRect.width() * m_titleSafePercent / 200.0);
    int titleH = static_cast<int>(targetRect.height() * m_titleSafePercent / 200.0);
    int actionW = static_cast<int>(targetRect.width() * m_actionSafePercent / 200.0);
    int actionH = static_cast<int>(targetRect.height() * m_actionSafePercent / 200.0);

    QPen titlePen(QColor(0, 255, 0, 150), 1, Qt::DashLine);
    QPen actionPen(QColor(255, 255, 0, 150), 1, Qt::DashLine);

    painter.setPen(actionPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(targetRect.adjusted(actionW, actionH, -actionW, -actionH));

    painter.setPen(titlePen);
    painter.drawRect(targetRect.adjusted(titleW, titleH, -titleW, -titleH));

    painter.setPen(QPen(QColor(0, 255, 0, 80), 1));
    int centerX = targetRect.center().x();
    int centerY = targetRect.center().y();
    painter.drawLine(centerX - 6, centerY, centerX + 6, centerY);
    painter.drawLine(centerX, centerY - 6, centerX, centerY + 6);
}

// ── Region of Interest ──────────────────────────────────────────

void CanvasWidget::setROIEnabled(bool enabled) {
    m_roiEnabled = enabled;
    update();
}

void CanvasWidget::setROI(int x, int y, int w, int h) {
    m_roiRect = QRect(x, y, w, h);
    update();
}

QRect CanvasWidget::applyROIClip(const QRect& targetRect) const {
    if (!m_roiEnabled || m_roiRect.isNull()) return targetRect;

    int compW = m_composition ? m_composition->getResolution().width : 1;
    int compH = m_composition ? m_composition->getResolution().height : 1;

    double scaleX = static_cast<double>(targetRect.width()) / compW;
    double scaleY = static_cast<double>(targetRect.height()) / compH;

    int rx = targetRect.x() + static_cast<int>(m_roiRect.x() * scaleX);
    int ry = targetRect.y() + static_cast<int>(m_roiRect.y() * scaleY);
    int rw = static_cast<int>(m_roiRect.width() * scaleX);
    int rh = static_cast<int>(m_roiRect.height() * scaleY);

    return QRect(rx, ry, rw, rh).intersected(targetRect);
}

void CanvasWidget::drawROI(QPainter& painter, const QRect& targetRect) {
    if (!m_roiEnabled) return;

    QRect roiScreen = applyROIClip(targetRect);

    painter.setPen(QPen(QColor(255, 128, 0, 200), 2, Qt::SolidLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(roiScreen);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 120));

    painter.drawRect(QRect(targetRect.left(), targetRect.top(),
                           targetRect.width(), roiScreen.top() - targetRect.top()));
    painter.drawRect(QRect(targetRect.left(), roiScreen.bottom() + 1,
                           targetRect.width(), targetRect.bottom() - roiScreen.bottom()));
    painter.drawRect(QRect(targetRect.left(), roiScreen.top(),
                           roiScreen.left() - targetRect.left(), roiScreen.height()));
    painter.drawRect(QRect(roiScreen.right() + 1, roiScreen.top(),
                           targetRect.right() - roiScreen.right(), roiScreen.height()));
}

// ── Motion Path ─────────────────────────────────────────────────

void CanvasWidget::drawMotionPath(QPainter& painter, const QRect& compRect) {
    if (m_selectedLayerIndex < 0 || !m_composition) return;

    auto layers = m_composition->getLayers();
    if (m_selectedLayerIndex >= static_cast<int>(layers.size())) return;

    auto layer = layers[m_selectedLayerIndex];
    const auto& posTrack = layer->getPosition();
    if (!posTrack.hasKeyframes()) return;

    int compW = m_composition->getResolution().width;
    int compH = m_composition->getResolution().height;

    double scaleX = static_cast<double>(compRect.width()) / compW;
    double scaleY = static_cast<double>(compRect.height()) / compH;
    double scale = std::min(scaleX, scaleY);

    double centerX = compRect.center().x();
    double centerY = compRect.center().y();

    const auto& keyframes = posTrack.getKeyframes();
    if (keyframes.size() < 2) return;

    QPen pathPen(QColor(255, 255, 0, 180), 1.5, Qt::SolidLine);
    painter.setPen(pathPen);
    painter.setBrush(Qt::NoBrush);

    QPoint prevScreen;
    bool hasPrev = false;

    for (size_t i = 0; i < keyframes.size(); i++) {
        double time = keyframes[i].getTime();
        double px = posTrack.getValueAtTime(time);
        double py = keyframes[i].getValue();

        int sx = static_cast<int>(centerX + px * scale);
        int sy = static_cast<int>(centerY + py * scale);

        QPoint currentScreen(sx, sy);

        if (hasPrev) {
            painter.drawLine(prevScreen, currentScreen);
        }

        painter.setPen(QPen(QColor(255, 255, 0), 3));
        painter.setBrush(QColor(255, 255, 0, 180));
        painter.drawEllipse(currentScreen, 3, 3);

        if (i == 0) {
            painter.setPen(QPen(QColor(0, 255, 0), 2));
            painter.setBrush(QColor(0, 255, 0, 180));
            painter.drawEllipse(currentScreen, 4, 4);
        }

        prevScreen = currentScreen;
        hasPrev = true;
    }
}

// ── Guides ──────────────────────────────────────────────────────

void CanvasWidget::addHorizontalGuide(double position) {
    m_horizontalGuides.push_back(position);
    update();
}

void CanvasWidget::addVerticalGuide(double position) {
    m_verticalGuides.push_back(position);
    update();
}

void CanvasWidget::removeGuide(int index) {
    if (index >= 0 && index < static_cast<int>(m_horizontalGuides.size())) {
        m_horizontalGuides.erase(m_horizontalGuides.begin() + index);
    } else {
        index -= static_cast<int>(m_horizontalGuides.size());
        if (index >= 0 && index < static_cast<int>(m_verticalGuides.size())) {
            m_verticalGuides.erase(m_verticalGuides.begin() + index);
        }
    }
    update();
}

void CanvasWidget::clearGuides() {
    m_horizontalGuides.clear();
    m_verticalGuides.clear();
    update();
}

void CanvasWidget::setSnapToGuides(bool snap) {
    m_snapToGuides = snap;
}

Point2D CanvasWidget::snapToGuides(Point2D point) const {
    if (!m_snapToGuides) return point;

    for (double gy : m_horizontalGuides) {
        if (std::abs(point.y - gy) < kSnapThreshold) {
            point.y = gy;
        }
    }
    for (double gx : m_verticalGuides) {
        if (std::abs(point.x - gx) < kSnapThreshold) {
            point.x = gx;
        }
    }
    return point;
}

double CanvasWidget::snapToGrid(double value, double gridSize) const {
    if (gridSize <= 0) return value;
    return std::round(value / gridSize) * gridSize;
}

void CanvasWidget::drawGuides(QPainter& painter, const QRect& targetRect) {
    if (m_horizontalGuides.empty() && m_verticalGuides.empty()) return;

    QPen guidePen(QColor(0, 200, 255, 150), 1, Qt::DashDotLine);
    painter.setPen(guidePen);

    for (double gy : m_horizontalGuides) {
        int sy = targetRect.top() + static_cast<int>(gy);
        painter.drawLine(targetRect.left(), sy, targetRect.right(), sy);
    }
    for (double gx : m_verticalGuides) {
        int sx = targetRect.left() + static_cast<int>(gx);
        painter.drawLine(sx, targetRect.top(), sx, targetRect.bottom());
    }
}

// ── Channel Overlay ─────────────────────────────────────────────

void CanvasWidget::drawChannelOverlay(QPainter& painter, const QRect& targetRect) {
    if (m_channelDisplay == ChannelDisplay::RGB) return;

    QString label;
    QColor labelColor;
    switch (m_channelDisplay) {
        case ChannelDisplay::Red:   label = "Red";   labelColor = QColor(255, 80, 80); break;
        case ChannelDisplay::Green: label = "Green"; labelColor = QColor(80, 255, 80); break;
        case ChannelDisplay::Blue:  label = "Blue";  labelColor = QColor(80, 80, 255); break;
        case ChannelDisplay::Alpha: label = "Alpha"; labelColor = QColor(200, 200, 200); break;
        default: return;
    }

    QRect labelRect(targetRect.left() + 8, targetRect.top() + 8, 80, 22);
    painter.fillRect(labelRect, QColor(0, 0, 0, 160));
    painter.setPen(labelColor);
    painter.setFont(QFont("SF Pro Display", 10, QFont::Bold));
    painter.drawText(labelRect, Qt::AlignCenter, label);
}

// ── Multi-View ──────────────────────────────────────────────────

void CanvasWidget::drawMultiView(QPainter& painter, const QRect& targetRect) {
    if (!m_multiViewEnabled || m_viewCount <= 1) return;

    int cols = (m_viewCount <= 2) ? m_viewCount : 2;
    int rows = (m_viewCount <= 1) ? 1 : (m_viewCount + 1) / 2;

    int cellW = targetRect.width() / cols;
    int cellH = targetRect.height() / rows;

    QPen borderPen(QColor(80, 80, 80), 1);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            if (idx >= m_viewCount) break;

            QRect cell(targetRect.x() + c * cellW,
                       targetRect.y() + r * cellH,
                       cellW, cellH);

            painter.drawRect(cell);

            ViewMode modes[] = {ViewMode::ActiveCamera, ViewMode::Top,
                                ViewMode::Front, ViewMode::Right};
            QString viewNames[] = {"Active Camera", "Top", "Front", "Right"};

            if (idx < 4) {
                QRect labelRect(cell.x() + 4, cell.y() + 4, 100, 18);
                painter.fillRect(labelRect, QColor(0, 0, 0, 140));
                painter.setPen(QColor(180, 180, 180));
                painter.setFont(QFont("SF Pro Display", 9));
                painter.drawText(labelRect, Qt::AlignVCenter | Qt::AlignLeft,
                                 " " + viewNames[idx]);
            }
        }
    }
}

// ── Preview Resolution ──────────────────────────────────────────

void CanvasWidget::setPreviewResolution(PreviewResolution res) {
    m_previewResolution = res;
    renderFrame();
    update();
}

double CanvasWidget::getPreviewScaleFactor() const {
    switch (m_previewResolution) {
        case PreviewResolution::Full:   return 1.0;
        case PreviewResolution::Half:   return 0.5;
        case PreviewResolution::Third:  return 1.0 / 3.0;
        case PreviewResolution::Quarter: return 0.25;
        case PreviewResolution::Auto: {
            if (m_zoom >= 2.0) return 0.5;
            if (m_zoom >= 4.0) return 0.25;
            return 1.0;
        }
    }
    return 1.0;
}

void CanvasWidget::drawPreviewResolutionLabel(QPainter& painter) {
    QString label;
    switch (m_previewResolution) {
        case PreviewResolution::Auto:    label = "Auto"; break;
        case PreviewResolution::Full:    label = "Full"; break;
        case PreviewResolution::Half:    label = "Half"; break;
        case PreviewResolution::Third:   label = "Third"; break;
        case PreviewResolution::Quarter: label = "Quarter"; break;
    }

    QRect labelRect(width() - 60, 6, 50, 18);
    painter.fillRect(labelRect, QColor(0, 0, 0, 160));
    painter.setPen(QColor(180, 180, 180));
    painter.setFont(QFont("SF Pro Display", 9));
    painter.drawText(labelRect, Qt::AlignCenter, label);
}

// ── Coordinate Helpers ──────────────────────────────────────────

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

// ── Paint Event ─────────────────────────────────────────────────

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
        QImage displayImage = applyChannelDisplay(m_frameImage);
        QRect drawTarget = targetRect;
        if (m_roiEnabled) {
            drawTarget = applyROIClip(targetRect);
        }
        painter.drawImage(drawTarget, displayImage);
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

    drawSafeMargins(painter, targetRect);
    drawROI(painter, targetRect);

    if (m_showGrid) drawGrid(painter, targetRect);

    drawMotionPath(painter, targetRect);
    drawGuides(painter, targetRect);

    drawSelectionRectangle(painter);

    drawLayerBoundingBox(painter, targetRect);

    if (m_selectedLayerIndex >= 0) {
        QRect layerRect = getSelectedLayerScreenRect();
        drawTransformHandles(painter, layerRect);
        drawRotationHandle(painter, layerRect);
    }

    if (m_multiViewEnabled && m_viewCount > 1) {
        drawMultiView(painter, targetRect);
    }

    drawChannelOverlay(painter, targetRect);
    drawPreviewResolutionLabel(painter);

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

        if (m_ramPreviewActive) {
            QRect previewRect(85, height() - 28, 80, 20);
            painter.fillRect(previewRect, QColor(180, 40, 40, 180));
            painter.setPen(QColor(255, 255, 255));
            painter.setFont(QFont("Menlo", 9));
            painter.drawText(previewRect, Qt::AlignCenter, "RAM PREVIEW");
        }
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

// ── Events ──────────────────────────────────────────────────────

void CanvasWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
}

void CanvasWidget::wheelEvent(QWheelEvent* event) {
    double delta = event->angleDelta().y() / 120.0;
    m_zoom = std::clamp(m_zoom + delta * 0.1, 0.1, 10.0);
    emit zoomChanged(m_zoom);
    update();
}

void CanvasWidget::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Space:
            if (m_ramPreviewActive) {
                stopRAMPreview();
            } else {
                startRAMPreview();
            }
            break;
        case Qt::Key_Right:
            stepForward();
            break;
        case Qt::Key_Left:
            stepBackward();
            break;
        case Qt::Key_Home:
            goToFirstFrame();
            break;
        case Qt::Key_End:
            goToLastFrame();
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
    }
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
